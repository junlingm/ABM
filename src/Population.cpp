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
  agent->_index = _agents.size();
  _agents.push_back(agent);
  schedule(agent);
  agent->_population = this;
  agent->report();
  for (auto c : _contacts)
    c->add(*agent);
  Simulation *sim = simulation();
  if (agent->_id == 0 && sim) agent->attached(*sim);
}

void Population::add(PContact contact)
{
  _contacts.push_back(contact);
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
    return NULL;
  for (auto &c : _contacts)
    c->remove(agent);
  agent._contactEvents->clearEvents();
  agent._population = nullptr;
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

void Population::attached(Simulation &sim)
{
  Agent::attached(sim);
  for (auto &a : _agents)
    a->attached(sim);
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
void addAgent(XP<Population> population, XP<Agent> agent)
{
  population->add(agent);
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

