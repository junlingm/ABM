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

CharacterVector Contact::classes = CharacterVector::create("Contact");

extern "C" {
  SEXP newRandomMixing(SEXP population)
  {
    return XP<RandomMixing>(std::make_shared<RandomMixing>(**XP<Population>(population)));
  }
}
