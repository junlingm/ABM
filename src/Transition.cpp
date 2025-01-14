#include "../inst/include/Simulation.h"

using namespace Rcpp;

#define DEBUG 0

#if DEBUG
#define PRINT(...) Rprintf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

using namespace Rcpp;

class TransitionEvent : public Event {
public:
  TransitionEvent(double time, Transition &rule);
  
  virtual bool handle(Simulation &sim, Agent &agent);
  
protected:
  Transition &_rule;
};

class ContactEvent : public Event {
public:
  ContactEvent(double time, PAgent contact, ContactTransition &rule);
  
  virtual bool handle(Simulation &sim, Agent &agent);
  
protected:
  ContactTransition &_rule;
  PAgent _contact;
};

WaitingTime::~WaitingTime()
{
}

TransitionEvent::TransitionEvent(double time, Transition &rule)
  : Event(time), _rule(rule)
{
}

Transition::~Transition()
{
}

bool TransitionEvent::handle(Simulation &sim, Agent &agent)
{
  double t = time();
  if (agent.match(_rule.from())) {
    if (_rule.toChange(t, agent)) {
      PRINT("%lf, NA, %d, NA, 1\n", t, agent.id());
      agent.set(_rule.to());
      _rule.changed(t, agent);
    }
  } else PRINT("%lf, NA, %d, NA, 0\n", t, agent.id());
  return false;
}

Transition::Transition(const List &from, const List &to, 
                       PWaitingTime waiting_time, 
                       Nullable<Function> to_change_callback, 
                       Nullable<Function> changed_callback)
  : _from(from), _to(to), _waiting_time(waiting_time)
{
  _to_change = to_change_callback.isNull() ? nullptr : new Function(to_change_callback);
  _changed = changed_callback.isNull()? nullptr : new Function(changed_callback);
}

bool Transition::toChange(double time, Agent &agent)
{
  if (_to_change == nullptr) return true;
  return as<bool>((*_to_change)(NumericVector::create(time), XP<Agent>(agent)));
}

void Transition::changed(double time, Agent &agent)
{
  if (_changed != nullptr)
    (*_changed)(NumericVector::create(time), XP<Agent>(agent));
}


void Transition::schedule(double time, Agent &agent)
{
  double t = _waiting_time->waitingTime(time) + time;
  PRINT("%lf, %lf, %ld, NA, NA\n", time, t, agent.id());
  agent.schedule(std::make_shared<TransitionEvent>(t, *this));
}

ContactEvent::ContactEvent(double time, PAgent contact, ContactTransition &rule)
  : Event(time), _rule(rule), _contact(contact)
{
}

bool ContactEvent::handle(Simulation &sim, Agent &agent)
{
  double t = time();
  if (agent.population() != _contact->population()) {
    PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
    return false;
  }
  if (agent.match(_rule.from())) {
    if (_contact->match(_rule.contactFrom()) && 
        _rule.toChange(t, agent, *_contact)) {
      PRINT("%lf, NA, %ld, %ld, 1\n", t, agent.id(), _contact->id());
      if (!agent.match(_rule.to()))
        agent.set(_rule.to());
      if (!_contact->match(_rule.contactTo()))
        _contact->set(_rule.contactTo());
      _rule.changed(t, agent, *_contact);
    } else PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
    _rule.schedule(t, agent);
  } else PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
  return false;
}

ContactTransition::ContactTransition(
  const Rcpp::List &agent_from, const Rcpp::List &contact_from, 
  const Rcpp::List &agent_to, const Rcpp::List &contact_to,
  Contact &contact,
  PWaitingTime waiting_time, 
  Rcpp::Nullable<Rcpp::Function> to_change_callback, 
  Rcpp::Nullable<Rcpp::Function> changed_callback)
  : Transition(agent_from, agent_to, waiting_time, to_change_callback, changed_callback),
    _contact_from(contact_from), _contact_to(contact_to), _contact(contact)
{
}

bool ContactTransition::toChange(double time, Agent &agent, Agent &contact)
{
  if (_to_change == nullptr) return true;
  return as<bool>((*_to_change)(NumericVector::create(time), XP<Agent>(agent), XP<Agent>(contact)));
}

void ContactTransition::changed(double time, Agent &agent, Agent &contact)
{
  if (_changed != nullptr)
    (*_changed)(NumericVector::create(time), XP<Agent>(agent), XP<Agent>(contact));
}

void ContactTransition::schedule(double time, Agent &agent)
{
  auto contact = _contact.contact(time, agent);
  if (contact.empty()) return;
  double waiting_time = R_PosInf;
  Agent* next_contact;
  for (auto c : contact) {
    double t = _waiting_time->waitingTime(time);
    if (t < waiting_time) {
      waiting_time = t;
      next_contact = c;
    }
  }
  if (waiting_time < R_PosInf) {
    PRINT("%lf, %lf, %ld, %ld, NA\n", time, waiting_time+time, agent.id(), next_contact->id());
    agent._contactEvents->schedule(std::make_shared<ContactEvent>(
        waiting_time + time, next_contact->population()->agent(*next_contact), *this));
  }
}

ExpWaitingTime::ExpWaitingTime(double rate)
  : _exp(rate)
{
}

double ExpWaitingTime::waitingTime(double time)
{
  return _exp.get();
}

GammaWaitingTime::GammaWaitingTime(double shape, double scale)
  : _shape(shape), _scale(scale)
{
}

double GammaWaitingTime::waitingTime(double time)
{
  RNGScope rngScope;
  return R::rgamma(_shape, _scale);
}

RWaitingTime::RWaitingTime(Function f)
  : _f(f)
{
}

double RWaitingTime::waitingTime(double time)
{
  return as<double>(_f(NumericVector::create(time)));
}

CharacterVector WaitingTime::classes = CharacterVector::create("WaitingTime");

CharacterVector Transition::classes = CharacterVector::create("Transition");

// [[Rcpp::export]]
XP<WaitingTime> newExpWaitingTime(double rate)
{
  return XP<WaitingTime>(std::make_shared<ExpWaitingTime>(rate));
}

// [[Rcpp::export]]
XP<WaitingTime> newGammaWaitingTime(double shape, double scale)
{
  return XP<WaitingTime>(std::make_shared<GammaWaitingTime>(shape, scale));
}

// [[Rcpp::export]]
XP<WaitingTime> newRWaitingTime(Function rng) 
{
  return XP<WaitingTime>(std::make_shared<RWaitingTime>(rng));
}

// [[Rcpp::export]]
double getWaitingTime(XP<WaitingTime> generator, double time)
{
  return generator->waitingTime(time);
}
