#include "../inst/include/Agent.h"
#include "../inst/include/Simulation.h"
#include <math.h>

using namespace Rcpp;

Agent::Agent(Nullable<List> state)
  : Calendar(), _population(nullptr), _contactEvents(new Calendar)
{
  if (state != R_NilValue) _state &= state.as();
  schedule(_contactEvents);
}

bool Agent::handle(Simulation &sim, Agent &agent)
{
  return Calendar::handle(sim, *this);
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

