#include "../inst/include/Population.h"

using namespace Rcpp;

Population::Population(size_t n, Nullable<Function> initializer)
  : Agent()
{
  if (n) _agents.reserve(n);
  if (initializer == R_NilValue) {
    for (size_t i = 0; i < n; ++i) {
      auto agent = std::make_shared<Agent>();
      add(agent);
    }
  } else {
    Function f(initializer.as());
    for (size_t i = 0; i < n; ++i) {
      SEXP s = f(i);
      if (!Rf_isList(s) && s != R_NilValue)
        s = List(s);
      auto agent = std::make_shared<Agent>(Nullable<List>(s));
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
    if (!Rf_isList(s) && s != R_NilValue)
      s = List(s);
    auto agent = std::make_shared<Agent>(s);
    add(agent);
  }
}

void Population::add(PAgent agent)
{
  if (agent->_population == this) return;
  if (_available.empty()) {
    _agents.push_back(agent);
    agent->_id = _agents.size();
  } else {
    size_t id = *_available.begin();
    agent->_id = id;
    _available.erase(_available.begin());
    _agents[id - 1] = agent;
  }
  schedule(agent);
  agent->_population = this;
  agent->report();
  for (auto c : _contacts)
    c->add(agent);
}

void Population::add(PContact contact)
{
  _contacts.push_back(contact);
  for (auto &a : _agents)
    contact->add(a);
}

PAgent Population::agentAtIndex(size_t i) const
{
  for (auto a : _available)
    if (a - 1 <= i) ++i;
  return _agents[i];
}

void Population::report()
{
  for (auto &c : _contacts)
    c->attach(*this);
  Agent::report();
  for (auto &a : _agents)
    a->report();
}

void Population::remove(Agent &agent)
{
  if (agent._population == this) {
    for (auto &c : _contacts)
      c->remove(agent);
    size_t id = agent._id;
    agent._contactEvents->clearEvents();
    agent._population = nullptr;
    _available.insert(id);
    unschedule(_agents[id - 1]);
    _agents[id - 1]= nullptr;
  }
}

CharacterVector Population::classes = CharacterVector::create("Population", "Agent", "Event");

// [[Rcpp::export]]
XP<Population> newPopulation(SEXP n, Nullable<Function> initializer = R_NilValue)
{
  if (n == R_NilValue)
    return XP<Population>(std::make_shared<Population>());
  if (Rf_isList(n))
    return XP<Population>(std::make_shared<Population>(List(n)));
  if (!Rf_isNumeric(n))
    stop("n must be an integer or a list");
  int N = as<int>(n); 
  if (N < 0) N = 0;
  return XP<Population>(std::make_shared<Population>(N, initializer));
}

// [[Rcpp::export]]
XP<Population> addAgent(XP<Population> population, XP<Agent> agent)
{
  population->add(agent);
  return population;
}

// [[Rcpp::export]]
int getSize(XP<Population> population)
{
  return population->size();
}
  
// [[Rcpp::export]]
XP<Agent> getAgent(XP<Population> population, int i)
{
  return population->agentAtIndex(i - 1);
}

// [[Rcpp::export]]
XP<Population> addContact(XP<Population> population, XP<Contact> contact)
{
  population->add(contact);
  return population;
}

// [[Rcpp::export]]
XP<Population> setStates(XP<Population> population, SEXP states)
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
  return population;
}

