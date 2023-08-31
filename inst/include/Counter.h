#pragma once

#include "Agent.h"
#include <string>

/**
 * An abstract class that represents logging state changes of agents.
 * When state changes occur, it is passed to each logger, which then
 * change its value. At the specified time points in a run, the
 * values of the logger are reported and recorded in a data.frame object,
 * where the columns represent variables, and rows represent the 
 * observation at each time point given to each run. Each logger has a 
 * name, which becomes the the column name int eh data.frame.
 */
class Logger {
public:
  /**
   * Constructor with a given name for the logger
   * 
   * @param name the name of the logger, which becomes the column name 
   * in the data.frame that is returned 
   */
  Logger(const std::string &name);
  /**
   * Destructor
   */
  virtual ~Logger();

  /**
   * Logs the state change of an agent
   * 
   * @param agent the agent whose state has just changed
   * 
   * @param from_state the original state of the agent bfore the change.
   */
  virtual void log(const Agent &agent, const State &from_state) = 0;
  /**
   * returns the current value of the logger
   */
  virtual long report() = 0;
  /**
   * the name of the logger
   */
  const std::string name() const { return _name; }
  
protected:
  /**
   * the name of the logger
   */
  std::string _name;
};

/**
 * A logger that counts the number of specific state transitions
 */
class Counter : public Logger {
public:
  /**
   * Constructor
   * 
   * @param name the name of the counter. See the class description for Logger.
   * 
   * @param state the state of the agent, please see the details
   * 
   * @param to the state of the agent after the state change. 
   * Can be R_NilValue. Please see the details.
   * 
   * @param initial the initial value of the counter. Default to 0.
   * 
   * @details if the argument to is not R_NilValue, then the counter 
   * counts the transitions from "state" to "to". Otherwise, it counts
   * the number of agents in a state that matches the "state" argument.
   * Specifically, if the agent jumps to "state", then the count increases
   * by 1. If the agents jumps away from "state", then the count decreases
   * by 1.
   */
  Counter(const std::string &name, const Rcpp::List &state, Rcpp::Nullable<Rcpp::List> to=R_NilValue, long initial = 0);
  
  /**
   * Log the state transition of the agent
   * 
   * @param agent the agent whose state has just changed
   * 
   * @param from the original state of the agent before transition
   */
  virtual void log(const Agent &agent, const State &from_state);
  
  /**
   * report the current state of the logger. 
   * 
   * @details if "to" is not R_NilValue, then the counter will be
   * reset to 0 after the report, so that it reports the number of 
   * transitions between report. Otherwise, it will not, so that it 
   * counts the number of agents in a specific state at the time of 
   * report.
   */
  virtual long report();
  
  /**
   * AThe classes of a Counter object.
   */
  static Rcpp::CharacterVector classes;

protected:
  /**
   * the current count
   */
  long _count;
  /**
   * The state to match. Please see the details in the class description
   */
  Rcpp::List _state;
  /**
   * The state that an agent jumps to. Please see the details in the 
   * class description
   */
  Rcpp::Nullable<Rcpp::List> _to;
};

/**
 * Log the specified state of a given agent
 * 
 * The state that can be logged must have a numeric value.
 */
class StateLogger : public Logger {
public:
  /**
   * Constructor
   * 
   * @param name the name of the logger. See the class description for Logger.
   * 
   * @param agent the agent whose state will be logged
   * 
   * @param state the state name of the state of the agent to be logged
   */
  StateLogger(const std::string &name, PAgent agent, const std::string &state);
  /**
   * Log the state transition of the agent
   * 
   * @param agent the agent whose state has just changed
   * 
   * @param from the original state of the agent before transition
   */
  virtual void log(const Agent &agent, const State &from_state);
  /**
   * report the current state of the logger. 
   */
  virtual long report();

  /**
   * the classes of StateLogger
   */
  static Rcpp::CharacterVector classes;

protected:
  /**
   * stores the value of the state that has been logged 
   */
  double _value;
  /**
   * the agent whose state will be logged. If R_NilValue, then
   * the current agent whose state has changed will be logged.
   */
  std::weak_ptr<Agent> _agent;
  /**
   * the namme of the state to be logged.
   */
  std::string _state;
};
