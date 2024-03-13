#include "../inst/include/Contact.h"
#include "../inst/include/Agent.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"

using namespace Rcpp;

Contact::Contact()
  : _population(nullptr)
{
}

Contact::~Contact()
{
}

void Contact::attach(Population &population)
{
  _population = &population;
  build();
}

RandomMixing::RandomMixing()
  : Contact(), _neighbors(1)
{
}

const std::vector<Agent*> &RandomMixing::contact(double time, Agent &agent)
{
  size_t n = _population->size();
  if (n <= 1)
    _neighbors.resize(0);
  else {
    while (true) {
      size_t i = _unif.get() * n;
      auto c = _population->agentAtIndex(i).get();
      if (c != &agent) {
        _neighbors[0] = c;
        break;
      }
    }
  }
  return _neighbors;
}

void RandomMixing::add(Agent &agent)
{
}

void RandomMixing::remove(Agent &agent)
{
}

void RandomMixing::build()
{
}

RContact::RContact(Environment r6)
  : Contact(), _r6(r6), 
    _r6_contact(r6["contact"]),
    _r6_addAgent(r6["addAgent"]),
    _r6_attach(r6["attach"]),
    _r6_remove(r6["remove"])
{
}

const std::vector<Agent*> &RContact::contact(double time, Agent &agent)
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

void RContact::add(Agent &agent)
{
  _r6_addAgent(XP<Agent>(agent));
}

void RContact::build()
{
  _r6_attach(XP<Population>(*_population));
}

void RContact::remove(Agent &agent)
{
  _r6_remove(XP<Agent>(agent));
}


CharacterVector Contact::classes = CharacterVector::create("Contact");

/**
 * Create an object of the RandomMixing class
 * 
 * @return an external pointer
 */
// [[Rcpp::export]]
XP<Contact> newRandomMixing()
{
  return XP<Contact>(std::make_shared<RandomMixing>());
}
  
/**
 * Create an RContact object
 * 
 * @param an R6Contact object
 * 
 * @return an external pointer
 * 
 * @details this is an internal method used by the R6Contact class
 */
// [[Rcpp::export]]
XP<Contact> newContact(Environment r6)
{
  return XP<Contact>(std::make_shared<RContact>(r6));
}
