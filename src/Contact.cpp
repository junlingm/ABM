#include "../inst/include/Contact.h"
#include "../inst/include/Agent.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"
#include "../inst/include/Transition.h"
#include <utility>

using namespace Rcpp;

Contact::Contact(std::string type, PWaitingTime waiting_time)
  : _population(nullptr), _type(std::move(type)),
    _waiting_time(std::move(waiting_time)),
    _explicit_rate(static_cast<bool>(_waiting_time))
{
  if (_type.empty())
    stop("contact type must not be empty");
}

void Contact::assignLegacyRate(PWaitingTime waiting_time)
{
  if (!waiting_time) return;
  if (_waiting_time)
    stop("contact rate is defined more than once");
  _waiting_time = std::move(waiting_time);
}

void Contact::resetLegacyRate()
{
  if (!_explicit_rate)
    _waiting_time.reset();
}

double Contact::waitingTime(double time) const
{
  return _waiting_time ? _waiting_time->waitingTime(time) : R_PosInf;
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

void Contact::detach(Population &population)
{
  if (_population == &population)
    _population = nullptr;
}

RandomMixing::RandomMixing(std::string type, PWaitingTime waiting_time)
  : Contact(std::move(type), std::move(waiting_time)), _neighbors(1)
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

RContact::RContact(Environment r6, std::string type,
                   PWaitingTime waiting_time)
  : Contact(std::move(type), std::move(waiting_time)),
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
  GenericVector c = contact(time, XP<Agent>(agent, agent.membershipLease()));
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
  addAgent(XP<Agent>(agent, agent.membershipLease()));
}

void RContact::build()
{
  Function attach = callback("attach");
  attach(XP<Population>(*_population, _population->lifetimeLease()));
}

void RContact::remove(Agent &agent)
{
  Function remove = callback("remove");
  remove(XP<Agent>(agent, agent.membershipLease()));
}


CharacterVector Contact::classes = CharacterVector::create("Contact");

static PWaitingTime parseWaitingTime(SEXP value)
{
  if (value == R_NilValue)
    return nullptr;
  if (TYPEOF(value) == EXTPTRSXP)
    return as<XP<WaitingTime> >(value);
  if (Rf_isFunction(value))
    return std::make_shared<RWaitingTime>(as<Function>(value));
  if (Rf_isNumeric(value))
    return std::make_shared<ExpWaitingTime>(as<double>(value));
  stop("contact rate must be a waiting-time object, function, number, or NULL");
}

/**
 * Create an object of the RandomMixing class
 * 
 * @return an external pointer
 */
// [[Rcpp::export]]
XP<Contact> newRandomMixing(SEXP rate = R_NilValue,
                            std::string type = "contact")
{
  return XP<Contact>(std::make_shared<RandomMixing>(
      std::move(type), parseWaitingTime(rate)));
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
XP<Contact> newContact(Environment r6, SEXP rate = R_NilValue,
                       std::string type = "contact")
{
  return XP<Contact>(std::make_shared<RContact>(
      r6, std::move(type), parseWaitingTime(rate)));
}

// [[Rcpp::export]]
std::string getContactType(XP<Contact> contact)
{
  return contact->type();
}

// [[Rcpp::export]]
bool isContactAttached(XP<Contact> contact)
{
  return contact->population() != nullptr;
}
