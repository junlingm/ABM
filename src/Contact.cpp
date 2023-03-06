#include "Contact.h"
#include "Agent.h"
#include "Population.h"
#include "RNG.h"

using namespace Rcpp;

RUnif unif;

Contact::Contact(Population &population)
  : _population(population)
{
}

Contact::~Contact()
{
}

RandomMixing::RandomMixing(Population &population)
  : Contact(population),   _neighbors(1)
{
}

const std::vector<PAgent> &RandomMixing::contact(double time, Agent &agent)
{
  size_t n = _population.size();
  size_t id = agent.id() - 1;
  size_t i = unif.get() * (n-1);
  size_t j = (i >= id) ? i + 1 : i;
  _neighbors[0] = _population.agent(j);
  return _neighbors;
}

void RandomMixing::add(const PAgent &agent)
{
}

void RandomMixing::finalize()
{
}

RContact::RContact(Population &population, Environment r6)
  : Contact(population), _r6(r6), 
    _r6_contact(r6["contact"]),
    _r6_addAgent(r6["addAgent"]),
    _r6_build(r6["build"])
{
}

const std::vector<PAgent> &RContact::contact(double time, Agent &agent)
{
  GenericVector c = _r6_contact(time, XP<Agent>(agent));
  size_t n = c.size();
  _neighbors.resize(n);
  for (size_t i = 0; i < n; ++i) {
    XP<Agent> a = c[i];
    _neighbors[i] = a;
  }
  return _neighbors;
}

void RContact::add(const PAgent &agent)
{
  _r6_addAgent(XP<Agent>(agent));
}

void RContact::finalize()
{
  _r6_build();
}

CharacterVector Contact::classes = CharacterVector::create("Contact");

extern "C" {
  SEXP newRandomMixing(SEXP population)
  {
    return XP<RandomMixing>(std::make_shared<RandomMixing>(**XP<Population>(population)));
  }
  
  SEXP newContact(SEXP population, SEXP contact)
  {
    XP<Population> p(population);
    return XP<RContact>(std::make_shared<RContact>(**p, contact));
  }
}
