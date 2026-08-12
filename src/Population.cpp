#include "../inst/include/Population.h"
#include <algorithm>

using namespace Rcpp;

Population::Population(size_t n, Nullable<Function> initializer)
  : Agent()
{
  if (n) _agents.reserve(n);
  if (initializer.isNull()) {
    for (size_t i = 0; i < n; ++i) {
      auto agent = makeOwned<Agent>();
      add(agent);
    }
  } else {
    Function f(initializer);
    for (size_t i = 0; i < n; ++i) {
      SEXP s = f(i + 1);
      if (!Rf_isNewList(s) && s != R_NilValue)
        s = List(s);
      auto agent = makeOwned<Agent>(Nullable<List>(s));
      add(agent);
    }
  }
}

Population::Population(List states)
  : Agent()
{
  size_t n = states.size();
  for (size_t i = 0; i < n; ++i) {
    SEXP s= states[i];
    if (!Rf_isNewList(s) && s != R_NilValue)
      s = List(s);
    auto agent = makeOwned<Agent>(Nullable<List>(s));
    add(agent);
  }
}

Population::~Population()
{
  for (auto &contact : _contacts)
    contact->detach(*this);
  for (auto &agent : _agents) {
    if (agent && agent->_population == this) {
      agent->_contactEvents->clearEvents();
      unschedule(agent);
      agent->_population = nullptr;
      agent->_membership_lease.reset();
    }
  }
}

void Population::add(PAgent agent)
{
  Population *nested = dynamic_cast<Population*>(agent.get());
  if (nested != nullptr) {
    for (Population *owner = this; owner != nullptr;
         owner = owner->_population) {
      if (owner == nested)
        stop("cannot add a population to itself or one of its descendants");
    }
  }
  if (agent->_population == this) return;
  if (agent->_population != nullptr)
    agent->leave();
  agent->_index = _agents.size();
  _agents.push_back(agent);
  schedule(agent);
  agent->_population = this;
  agent->registered(*this);
  agent->report();
  for (auto c : _contacts)
    c->add(*agent);
}

void Population::add(PContact contact)
{
  for (const auto &existing : _contacts)
    if (existing == contact)
      return;
  _contacts.push_back(contact);
  if (_population != nullptr)
    _population->registerSubcontact(contact);
  for (auto &a : _agents)
    contact->add(*a);
}

void Population::report()
{
  for (auto &c : _contacts)
    c->attach(*this);
  Agent::report();
  for (auto &a : _agents)
    a->report();
}

PAgent Population::remove(Agent &agent)
{
  if (agent._population != this) 
    stop("agent is not managed by this population");
  for (auto &c : _contacts)
    c->remove(agent);
  agent._contactEvents->clearEvents();
  agent.deregistered(*this);
  agent._population = nullptr;
  agent._membership_lease.reset();
  unsigned int i = agent._index;
  agent._index = 0;
  size_t n = _agents.size();
  PAgent a = _agents[i];
  if (n > 1 && i < n - 1) {
    _agents[i] = _agents[n - 1];
    _agents[n - 1] = nullptr;
    _agents[i]->_index = i;
  } else _agents[i]= nullptr;
  _agents.resize(n - 1);
  unschedule(a);
  return a;
}

void Population::setID(Simulation &sim)
{
  Agent::setID(sim);
  for (auto &a : _agents)
    a->setID(sim);
}

void Population::registered(Population &owner)
{
  Simulation *sim = owner.simulation();
  if (sim != nullptr)
    setID(*sim);
  for (const auto &contact : _contacts)
    owner.registerSubcontact(contact);
  for (const auto &contact : _subcontacts)
    owner.registerSubcontact(contact);
}

void Population::deregistered(Population &owner)
{
  for (const auto &contact : _contacts)
    owner.deregisterSubcontact(contact);
  for (const auto &contact : _subcontacts)
    owner.deregisterSubcontact(contact);
}

void Population::registerSubcontact(const PContact &contact)
{
  _subcontacts.push_back(contact);
  if (_population != nullptr)
    _population->registerSubcontact(contact);
}

void Population::deregisterSubcontact(const PContact &contact)
{
  auto position = std::find(_subcontacts.begin(), _subcontacts.end(), contact);
  if (position == _subcontacts.end())
    stop("subpopulation contact is not registered");
  _subcontacts.erase(position);
  if (_population != nullptr)
    _population->deregisterSubcontact(contact);
}

CharacterVector Population::classes = CharacterVector::create("Population", "Agent", "Event");

// [[Rcpp::export]]
XP<Population> newPopulation(SEXP n, Nullable<Function> initializer = R_NilValue)
{
  if (n == R_NilValue)
    return XP<Population>(makeOwned<Population>());
  if (Rf_isNewList(n))
    return XP<Population>(makeOwned<Population>(List(n)));
  if (!Rf_isNumeric(n))
    stop("n must be an integer or a list");
  int N = as<int>(n); 
  if (N < 0) N = 0;
  return XP<Population>(makeOwned<Population>(N, initializer));
}

// [[Rcpp::export]]
void addAgent(XP<Population> population, XP<Agent> agent)
{
  Agent *raw = agent;
  PAgent managed = agent;
  if (!managed && raw->population() != nullptr)
    managed = PAgent(raw);
  if (!managed)
    stop("agent is not managed by R or a population");
  population->add(managed);
}

// [[Rcpp::export]]
int getSize(XP<Population> population)
{
  return population->size();
}
  
// [[Rcpp::export]]
XP<Agent> getAgent(XP<Population> population, int i)
{
  if (i < 1 || static_cast<size_t>(i) > population->size())
    stop("agent index is out of range");
  return XP<Agent>(PAgent(population->agentAtIndex(i - 1)));
}

// [[Rcpp::export]]
void addContact(XP<Population> population, XP<Contact> contact)
{
  population->add(contact);
}

// [[Rcpp::export]]
void setStates(XP<Population> population, SEXP states)
{
  if (Rf_isFunction(states)) {
    Function f(states);
    size_t n = population->size();
    for (size_t i = 0; i < n; ++i)
      population->agentAtIndex(i)->set(f(i+1));
  } else if (Rf_isVector(states)) {
    List l(states);
    size_t n = l.size();
    if (n != population->size())
      stop("The length of the states and the population size must agree");
    for (size_t i = 0; i < n; ++i) {
      SEXP s = l[i];
      if (!Rf_isVector(s))
        s = List(s);
      population->agentAtIndex(i)->set(s);
    }
  } else stop("invalid states. Must be a function or a list");
}
