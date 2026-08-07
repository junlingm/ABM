#pragma once

#include <Rcpp.h>
#include <memory>
#include <string>

class Agent;
class ContactEvent;
class Simulation;
class TransitionEvent;

/**
 * An operation that is invoked after a transition has succeeded.
 *
 * Event loggers update state held by the simulation. They are distinct from
 * report loggers, which only read values at requested report times.
 */
class EventLogger {
public:
  virtual ~EventLogger();

  virtual void log(
      Simulation &simulation, Agent &agent, TransitionEvent &event) = 0;
  virtual void log(
      Simulation &simulation, Agent &agent, ContactEvent &event) = 0;

  static Rcpp::CharacterVector classes;
};

typedef std::shared_ptr<EventLogger> PEventLogger;

/**
 * An event logger that changes a numeric simulation state variable.
 */
class StateEventLogger : public EventLogger {
public:
  StateEventLogger(
      const std::string &variable,
      double change,
      Rcpp::Nullable<Rcpp::Function> filter = R_NilValue);

  virtual void log(
      Simulation &simulation, Agent &agent, TransitionEvent &event);
  virtual void log(
      Simulation &simulation, Agent &agent, ContactEvent &event);

protected:
  void apply(Simulation &simulation, Agent &agent);
  bool matches(const Agent &agent) const;

  std::string _variable;
  double _change;
  Rcpp::Nullable<Rcpp::Function> _filter;
};
