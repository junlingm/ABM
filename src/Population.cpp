#include "../inst/include/Population.h"

using namespace Rcpp;

Population::Population(size_t n)
  : Agent()
{
  if (n) _agents.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    auto agent = std::make_shared<Agent>();
    add(agent);
  }
}

void Population::add(PAgent agent)
{
  _agents.push_back(agent);
  agent->_id = _agents.size();
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

void Population::report()
{
  for (auto &c : _contacts)
    c->attach(*this);
  Agent::report();
  for (auto &a : _agents)
    a->report();
}

CharacterVector Population::classes = CharacterVector::create("Population", "Agent", "Event");

// [[Rcpp::export]]
XP<Population> newPopulation(int n = 0)
{
  if (n < 0) n = 0;
  return XP<Population>(std::make_shared<Population>(n));
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
  return population->agent(i - 1);
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
      population->agent(i)->set(f(i+1));
  } else if (Rf_isVector(states)) {
    List l(states);
    size_t n = l.size();
    if (n != population->size())
      stop("The length of the states and the population size must agree");
    for (size_t i = 0; i < n; ++i)
      population->agent(i)->set(l[i]);
  } else stop("invalid states. Must be a function or a list");
  return population;
}

