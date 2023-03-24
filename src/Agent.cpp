#include "../inst/include/Agent.h"
#include "../inst/include/Simulation.h"
#include <math.h>

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
  : Calendar(), _population(nullptr), _contactEvents(new Calendar)
{
  if (state != R_NilValue) _state &= state.as();
  schedule(_contactEvents);
}

bool Agent::handle(Simulation &sim, Agent &agent)
{
  Calendar::handle(sim, *this);
  return _population != nullptr;
}

void Agent::set(const Rcpp::List &state)
{
  State from(clone(_state));
  _state &= state;
  stateChanged(*this, from);
}

bool Agent::match(const Rcpp::List &state) const
{
  return _state.match(state);
}

void Agent::stateChanged(Agent &agent, const State &from)
{
  if (_population != nullptr)
    _population->stateChanged(agent, from);
}

void Agent::leave()
{
  if (_population != nullptr) {
    State save = _state;
    _state = State();
    stateChanged(*this, save);
    _population->remove(*this);
    _state = save;
  }
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
  if (!isnan(d)) a->setDeathTime(d);
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
XP<Agent> schedule(XP<Agent> agent, XP<Event> event)
{
  agent->schedule(event);
  return agent;
}

// [[Rcpp::export]]
XP<Agent> unschedule(XP<Agent> agent, XP<Event> event)
{
  agent->unschedule(event);
  return agent;
}

// [[Rcpp::export]]
XP<Agent> clearEvents(XP<Agent> agent)
{
  agent->clearEvents();
  return agent;
}

// [[Rcpp::export]]
XP<Agent> setState(XP<Agent> agent, SEXP value)
{
  Nullable<List> s(value);
  if (!s.isNull())
    agent->set(s.as());
  return agent;
}

// [[Rcpp::export]]
XP<Agent> leave(XP<Agent> agent)
{
  agent->leave();
  return agent;
}

// [[Rcpp::export]]
XP<Agent> setDeathTime(XP<Agent> agent, double time)
{
  agent->setDeathTime(time);
  return agent;
}

