#include "../inst/include/Simulation.h"

using namespace Rcpp;

EventLogger::~EventLogger()
{
}

StateEventLogger::StateEventLogger(
    const std::string &variable,
    double change,
    Nullable<Function> filter)
  : _variable(variable), _change(change), _filter(filter)
{
}

bool StateEventLogger::matches(const Agent &agent) const
{
  if (_filter.isNull()) return true;
  Function filter(_filter);
  return as<bool>(filter(agent.state()));
}

void StateEventLogger::apply(Simulation &simulation, Agent &agent)
{
  if (matches(agent)) simulation.change(_variable, _change);
}

void StateEventLogger::log(
    Simulation &simulation, Agent &agent, TransitionEvent &event)
{
  apply(simulation, agent);
}

void StateEventLogger::log(
    Simulation &simulation, Agent &agent, ContactEvent &event)
{
  apply(simulation, agent);
}

// [[Rcpp::export]]
XP<EventLogger> newIncrementLogger(
    std::string variable, Nullable<Function> filter = R_NilValue)
{
  return XP<EventLogger>(
      std::make_shared<StateEventLogger>(variable, 1, filter));
}

// [[Rcpp::export]]
XP<EventLogger> newDecrementLogger(
    std::string variable, Nullable<Function> filter = R_NilValue)
{
  return XP<EventLogger>(
      std::make_shared<StateEventLogger>(variable, -1, filter));
}

Rcpp::CharacterVector EventLogger::classes =
  Rcpp::CharacterVector::create("EventLogger");
