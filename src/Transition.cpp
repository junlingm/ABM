#include "../inst/include/Simulation.h"
#include <utility>

using namespace Rcpp;

#define DEBUG 0

#if DEBUG
#define PRINT(...) Rprintf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

using namespace Rcpp;

WaitingTime::~WaitingTime()
{
}

TransitionEvent::TransitionEvent(double time, Transition &rule)
  : Event(time), _rule(rule)
{
}

Transition::~Transition() = default;

bool TransitionEvent::handle(Simulation &sim, Agent &agent)
{
  double t = time();
  if (agent.match(_rule.from())) {
    if (_rule.toChange(t, agent)) {
      PRINT("%lf, NA, %d, NA, 1\n", t, agent.id());
      agent.set(_rule.to());
      _rule.log(sim, *this, agent);
      _rule.changed(t, agent);
    }
  } else PRINT("%lf, NA, %d, NA, 0\n", t, agent.id());
  return false;
}

TransitionRule::TransitionRule(
    const List &from, const List &to,
    Nullable<Function> to_change_callback,
    Nullable<Function> changed_callback,
    const std::vector<PEventLogger> &logging)
  : _from(from), _to(to), _logging(logging)
{
  if (!to_change_callback.isNull())
    _to_change.reset(new Function(to_change_callback));
  if (!changed_callback.isNull())
    _changed.reset(new Function(changed_callback));
}

Transition::Transition(const List &from, const List &to,
                       PWaitingTime waiting_time,
                       Nullable<Function> to_change_callback,
                       Nullable<Function> changed_callback,
                       const std::vector<PEventLogger> &logging)
  : TransitionRule(from, to, to_change_callback, changed_callback, logging),
    _waiting_time(waiting_time)
{
}

bool Transition::toChange(double time, Agent &agent)
{
  if (_to_change == nullptr) return true;
  PXPLease lease = std::make_shared<XPLease>();
  return as<bool>((*_to_change)(
    NumericVector::create(time), XP<Agent>(agent, lease)));
}

void Transition::changed(double time, Agent &agent)
{
  if (_changed != nullptr) {
    PXPLease lease = std::make_shared<XPLease>();
    (*_changed)(NumericVector::create(time), XP<Agent>(agent, lease));
  }
}

void Transition::log(Simulation &simulation, TransitionEvent &event, Agent &agent)
{
  for (auto &logger : _logging)
    logger->log(simulation, agent, event);
}

void Transition::schedule(double time, Agent &agent)
{
  double wait_time = _waiting_time->waitingTime(time);
  PRINT("%lf, %lf, %ld, NA, NA\n", time, wait_time, agent.id());
  if (wait_time < R_PosInf)
    agent.schedule(makeOwned<TransitionEvent>(time + wait_time, *this));
}

ContactEvent::ContactEvent(double time, Agent &contact, Contact &source,
                           ContactTransition &rule)
  : Event(time), _rule(rule), _source(source), _contact(&contact),
    _contact_lease(contact.membershipLease())
{
}

bool ContactEvent::handle(Simulation &sim, Agent &agent)
{
  double t = time();
  if (_contact_lease.expired())
    return false;
  Population *owner = agent.population();
  if (owner == nullptr || owner != _contact->population() ||
      owner != _source.population()) {
    PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
    return false;
  }
  if (agent.match(_rule.from())) {
    bool left_from = false;
    if (_contact->match(_rule.contactFrom()) && 
        _rule.toChange(t, agent, *_contact)) {
      PRINT("%lf, NA, %ld, %ld, 1\n", t, agent.id(), _contact->id());
      if (!agent.match(_rule.to())) {
        agent.set(_rule.to());
        left_from = !agent.match(_rule.from());
      }
      if (!_contact->match(_rule.contactTo()))
        _contact->set(_rule.contactTo());
      _rule.log(sim, *this, agent);
      _rule.changed(t, agent, *_contact);
    } else PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
    if (!left_from)
      _rule.schedule(t, agent, _source);
  } else PRINT("%lf, NA, %ld, %ld, 0\n", t, agent.id(), _contact->id());
  return false;
}

ContactTransition::ContactTransition(
  const Rcpp::List &agent_from, const Rcpp::List &contact_from, 
  const Rcpp::List &agent_to, const Rcpp::List &contact_to,
  std::optional<std::string> contact_type,
  PWaitingTime waiting_time, 
  Rcpp::Nullable<Rcpp::Function> to_change_callback, 
  Rcpp::Nullable<Rcpp::Function> changed_callback,
  const std::vector<PEventLogger> &logging)
  : TransitionRule(agent_from, agent_to, to_change_callback,
                   changed_callback, logging),
    _contact_from(contact_from), _contact_to(contact_to),
    _contact_type(std::move(contact_type)), _waiting_time(waiting_time)
{
}

bool ContactTransition::toChange(double time, Agent &agent, Agent &contact)
{
  if (_to_change == nullptr) return true;
  PXPLease lease = std::make_shared<XPLease>();
  return as<bool>((*_to_change)(
    NumericVector::create(time),
    XP<Agent>(agent, lease),
    XP<Agent>(contact, lease)));
}

void ContactTransition::changed(double time, Agent &agent, Agent &contact)
{
  if (_changed != nullptr) {
    PXPLease lease = std::make_shared<XPLease>();
    (*_changed)(
      NumericVector::create(time),
      XP<Agent>(agent, lease),
      XP<Agent>(contact, lease));
  }
}

void ContactTransition::log(
    Simulation &simulation, ContactEvent &event, Agent &agent)
{
  for (auto &logger : _logging)
    logger->log(simulation, agent, event);
}

bool ContactTransition::matches(const Contact &contact) const
{
  return !_contact_type || contact.type() == *_contact_type;
}

void ContactTransition::schedule(
    double time, Agent &agent, Contact &source)
{
  if (!matches(source)) return;
  if (source.population() != agent.population()) return;
  const auto &contact = source.contact(time, agent);
  if (contact.empty()) return;
  double waiting_time = R_PosInf;
  Agent* next_contact = nullptr;
  for (auto c : contact) {
    double t = source.waitingTime(time);
    if (t < waiting_time) {
      waiting_time = t;
      next_contact = c;
    }
  }
  if (waiting_time < R_PosInf) {
    if (next_contact == nullptr)
      stop("contact returned a null agent");
    PRINT("%lf, %lf, %ld, %ld, NA\n", time, waiting_time+time, agent.id(), next_contact->id());
    Agent *managed = source.population()->agent(*next_contact);
    if (!managed)
      stop("contact returned an agent not managed by its population");
    agent._contactEvents->schedule(makeOwned<ContactEvent>(
        waiting_time + time, *managed, source, *this));
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
  : _gamma(shape, 1 / scale)
{
}

double GammaWaitingTime::waitingTime(double time)
{
  return _gamma.get();
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
  return XP<WaitingTime>(makeOwned<ExpWaitingTime>(rate));
}

// [[Rcpp::export]]
XP<WaitingTime> newGammaWaitingTime(double shape, double scale)
{
  return XP<WaitingTime>(makeOwned<GammaWaitingTime>(shape, scale));
}

// [[Rcpp::export]]
XP<WaitingTime> newRWaitingTime(Function rng) 
{
  return XP<WaitingTime>(makeOwned<RWaitingTime>(rng));
}

// [[Rcpp::export]]
double getWaitingTime(XP<WaitingTime> generator, double time)
{
  return generator->waitingTime(time);
}
