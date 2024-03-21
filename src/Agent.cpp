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

PAgent Agent::leave()
{
  if (_population == nullptr) 
    return nullptr;
  State save = _state;
  _state = State();
  stateChanged(*this, save);
  PAgent agent = _population->remove(*this);
  _state = save;
  return agent;
}

void Agent::attached(Simulation &sim)
{
  if (_id == 0)
    _id = sim.nextID();
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
  if (!s.isNull())
    agent->set(s.as());
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

