#include "Population.h"
#include "State.h"

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
  for (auto a : _agents)
    contact->add(a);
}

void Population::report()
{
  for (auto c : _contacts)
    c->finalize();
  Agent::report();
  for (auto a : _agents)
    a->report();
}

CharacterVector Population::classes = CharacterVector::create("Population", "Agent", "Event");

extern "C" {
  SEXP newPopulation(SEXP n)
  {
    int m = (n == R_NilValue) ? 0 : as<int>(n);
    if (m < 0) m = 0;
    return XP<Population>(std::make_shared<Population>(m));
  }
  
  SEXP addAgent(SEXP population, SEXP agent)
  {
    XP<Population>(population)->add(XP<Agent>(agent));
    return population;
  }

  SEXP getSize(SEXP population)
  {
    return wrap(XP<Population>(population)->size());
  }
  
  SEXP getAgent(SEXP population, SEXP i)
  {
    int pos = as<int>(i) - 1;
    return XP<Agent>(XP<Population>(population)->agent(pos));
  }

  SEXP addContact(SEXP population, SEXP contact)
  {
    XP<Population>(population)->add(XP<Contact>(contact));
    return population;
  }
}