#include "../inst/include/Contact.h"
#include "../inst/include/Agent.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"
#include "../inst/include/Transition.h"
#include <utility>

using namespace Rcpp;

Contact::Contact(std::string type)
  : _population(nullptr), _type(std::move(type))
{
  if (_type.empty())
    stop("contact type must not be empty");
}

Contact::~Contact()
{
}

void Contact::attach(Population &population)
{
  if (_population == &population)
    return;
  if (_population != nullptr)
    stop("contact is already attached to a different population");
  _population = &population;
  build();
}

void Contact::addTransition(ContactTransition &transition)
{
  for (auto registered : _transitions)
    if (registered == &transition)
      return;
  _transitions.push_back(&transition);
}

void Contact::clearTransitions()
{
  _transitions.clear();
}

void Contact::schedule(double time, Agent &agent, const State &from)
{
  for (auto transition : _transitions)
    if (!from.match(transition->from()) && agent.match(transition->from()))
      transition->schedule(time, agent, *this);
}

void Contact::schedule(double time, Agent &agent,
                       ContactTransition &transition)
{
  for (auto registered : _transitions)
    if (registered == &transition) {
      transition.schedule(time, agent, *this);
      return;
    }
}

RandomMixing::RandomMixing(std::string type)
  : Contact(std::move(type)), _neighbors(1)
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

RContact::RContact(Environment r6, std::string type)
  : Contact(std::move(type)),
    _r6(R_MakeWeakRef(r6, R_NilValue, R_NilValue, FALSE))
{
}

Environment RContact::object() const
{
  SEXP key = _r6.key();
  if (key == R_NilValue)
    stop("R contact object is no longer available");
  return Environment(key);
}

Function RContact::callback(const char *name) const
{
  Environment r6 = object();
  return Function(r6[name]);
}

const std::vector<Agent*> &RContact::contact(double time, Agent &agent)
{
  Function contact = callback("contact");
  GenericVector c = contact(time, XP<Agent>(agent));
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
  Function addAgent = callback("addAgent");
  addAgent(XP<Agent>(agent));
}

void RContact::build()
{
  Function attach = callback("attach");
  attach(XP<Population>(*_population));
}

void RContact::remove(Agent &agent)
{
  Function remove = callback("remove");
  remove(XP<Agent>(agent));
}


CharacterVector Contact::classes = CharacterVector::create("Contact");

/**
 * Create an object of the RandomMixing class
 * 
 * @return an external pointer
 */
// [[Rcpp::export]]
XP<Contact> newRandomMixing(std::string type = "contact")
{
  return XP<Contact>(std::make_shared<RandomMixing>(std::move(type)));
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
XP<Contact> newContact(Environment r6, std::string type = "contact")
{
  return XP<Contact>(std::make_shared<RContact>(r6, std::move(type)));
}

// [[Rcpp::export]]
std::string getContactType(XP<Contact> contact)
{
  return contact->type();
}
