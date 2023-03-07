#include "../inst/include/Contact.h"
#include "../inst/include/Agent.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"

using namespace Rcpp;

RUnif unif;

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

const std::vector<PAgent> &RandomMixing::contact(double time, Agent &agent)
{
  size_t n = _population->size();
  size_t id = agent.id() - 1;
  size_t i = unif.get() * (n-1);
  size_t j = (i >= id) ? i + 1 : i;
  _neighbors[0] = _population->agent(j);
  return _neighbors;
}

void RandomMixing::add(const PAgent &agent)
{
}

void RandomMixing::build()
{
}

RContact::RContact(Environment r6)
  : Contact(), _r6(r6), 
    _r6_contact(r6["contact"]),
    _r6_addAgent(r6["addAgent"]),
    _r6_attach(r6["attach"])
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

void RContact::build()
{
  _r6_attach(XP<Population>(*_population));
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
