#include "../inst/include/Agent.h"
#include "../inst/include/Simulation.h"
#include <math.h>

using namespace Rcpp;

Agent::Agent(Nullable<List> state)
  : Event(R_PosInf), _population(nullptr)
{
  if (state != R_NilValue) _state &= state.as();
}

Agent::~Agent()
{
}

bool Agent::handle(Simulation &sim, Agent &agent)
{
  if (!_events.empty()) {
    auto e = _events.begin()->second;
    unschedule(e);
    if (e->handle(sim, *this)) {
      schedule(e);
    }
  }
  return true;
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

void Agent::schedule(PEvent event)
{
  if (event->_owner != nullptr)
    event->_owner->unschedule(event);
  double t = event->time();
  bool update = _time > t;
  if (update) _time = t;
  Agent *owner = update ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  event->_owner = this;
  event->_pos = _events.emplace(t, event);
  if (owner != nullptr)
    owner->schedule(me);
}

void Agent::unschedule(PEvent event)
{
  if (event == NULL || event->_owner != this) return;
  Agent *owner = (_time == event->time()) ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  _events.erase(event->_pos);
  event->_owner = nullptr;
  _time = _events.empty() ? R_PosInf : _events.begin()->first;
  if (owner != nullptr)
    owner->schedule(me);
}

void Agent::clearEvents()
{
  Agent *owner = !isinf(_time) ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  for (auto e : _events)
    e.second->_owner = NULL;
  _events.clear();
  _time = R_PosInf;
  if (owner != nullptr)
    owner->schedule(me);
}

static State empty_state;

void Agent::report()
{
  stateChanged(*this, empty_state);
}

CharacterVector Agent::classes = CharacterVector::create("Agent", "Event");

// [[Rcpp::export]]
XP<Agent> newAgent(Nullable<List> state)
{
  return XP<Agent>(std::make_shared<Agent>(state));
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

