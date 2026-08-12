#include "../inst/include/Agent.h"
#include "../inst/include/Simulation.h"
#include <cmath>

using namespace Rcpp;

class DeathEvent : public Event {
public:
  DeathEvent(double time) : Event(time) { }
  virtual bool handle(Simulation & sim, Agent &agent) {
    agent.leave();
    return false;
  }
};

Agent::Agent(Nullable<List> state)
  : Calendar(), _population(nullptr), _id(0), _index(0), _contactEvents(new Calendar)
{
  if (state.isNotNull()) _state &= List(state);
  schedule(_contactEvents);
}

bool Agent::handle(Simulation &sim, Agent &agent)
{
  Population *owner = _population;
  Calendar::handle(sim, *this);
  return owner != nullptr && _population == owner;
}

void Agent::set(const Rcpp::List &state, bool notify)
{
  if (notify)
    stateChanging(*this, state);
  _state &= state;
  if (notify)
    stateChanged(*this);
}

bool Agent::match(const Rcpp::List &state) const
{
  return _state.match(state);
}

void Agent::stateChanging(Agent &agent, const Rcpp::List &state)
{
  if (_population != nullptr)
    _population->stateChanging(agent, state);
}

void Agent::stateChanged(Agent &agent)
{
  if (_population != nullptr)
    _population->stateChanged(agent);
}

void Agent::stateChanged(Agent &agent, const State &from)
{
  if (_population != nullptr)
    _population->stateChanged(agent, from);
}

PAgent Agent::leave()
{
  Population *owner = _population;
  if (owner == nullptr)
    stop("agent is not attached to a population");
  State save = _state;
  stateChanging(*this, State());
  _state = State();
  stateChanged(*this);
  if (_population != owner) {
    _state = save;
    stop("agent changed populations while leaving");
  }
  PAgent agent;
  try {
    agent = owner->remove(*this);
  } catch (...) {
    _state = save;
    throw;
  }
  _state = save;
  if (!agent)
    stop("agent could not be removed from its population");
  return agent;
}

const PXPLease &Agent::membershipLease()
{
  if (_population == nullptr)
    stop("cannot borrow an agent without a population membership");
  if (!_membership_lease)
    _membership_lease = std::make_shared<XPLease>();
  return _membership_lease;
}

void Agent::setID(Simulation &sim)
{
  if (_id == 0)
    _id = sim.nextID();
}

void Agent::registered(Population &owner)
{
  if (_id != 0)
    return;
  Simulation *sim = owner.simulation();
  if (sim != nullptr)
    setID(*sim);
}

void Agent::deregistered(Population &owner)
{
}

Simulation *Agent::simulation()
{
  return _population == nullptr ? nullptr : _population->simulation();
}

const Simulation *Agent::simulation() const
{
  return _population == nullptr ? nullptr : _population->simulation();
}

void Agent::setDeathTime(double time)
{
  schedule(std::make_shared<DeathEvent>(time));
}

static State empty_state;

void Agent::report()
{
  stateChanged(*this, empty_state);
}

CharacterVector Agent::classes = CharacterVector::create("Agent", "Event");

// [[Rcpp::export]]
XP<Agent> newAgent(Nullable<List> state, NumericVector death_time = NA_REAL)
{
  XP<Agent> a = (std::make_shared<Agent>(state));
  double d = as<double>(death_time);
  if (!std::isnan(d)) a->setDeathTime(d);
  return a;
}
  
// [[Rcpp::export]]
int getID(XP<Agent> agent) 
{
  return agent->id();
}

// [[Rcpp::export]]
List getState(XP<Agent> agent)
{
  return agent->state();
}
  
// [[Rcpp::export]]
void schedule(XP<Agent> agent, XP<Event> event)
{
  agent->schedule(event);
}

// [[Rcpp::export]]
void unschedule(XP<Agent> agent, XP<Event> event)
{
  agent->unschedule(event);
}

// [[Rcpp::export]]
void clearEvents(XP<Agent> agent)
{
  agent->clearEvents();
}

// [[Rcpp::export]]
void setState(XP<Agent> agent, SEXP value)
{
  Nullable<List> s(value);
  if (s.isNotNull()) agent->set(List(s));
}

// [[Rcpp::export]]
XP<Agent> leave(XP<Agent> agent)
{
  return agent->leave();
}

// [[Rcpp::export]]
void setDeathTime(XP<Agent> agent, double time)
{
  agent->setDeathTime(time);
}
