#pragma once

#include "Population.h"
#include "Counter.h"
#include "Transition.h"
#include <list>
#include <map>

class Simulation : public Population {
public:
  Simulation(size_t n = 0);
  virtual ~Simulation();

  virtual Rcpp::List run(const Rcpp::NumericVector &time);
  virtual Rcpp::List resume(const Rcpp::NumericVector &time);
  
  void add(std::shared_ptr<Logger> counter);
  void add(Transition *rule);
  using Population::add;
  
  static Rcpp::CharacterVector classes;
  
protected:
  virtual void stateChanged(Agent &agent, const State &from);
  
  std::list<std::shared_ptr<Logger> > _loggers;
  std::list<Transition*> _rules;
  double _current_time;
};

extern "C" {
  /**
   * Create a new simulation
   * 
   * @param n the initial population size, a nonnegative integer or
   * NULL (meaning 0)
   * 
   * @return an external pointer pointing to the Simulation object.
   * 
   * @details if n is not 0 or NULL, then n agents initially population
   * the simulation with empty state.
   */
  SEXP newSimulation(SEXP n);
  
  /**
   * Run the simulation
   * 
   * @param sim the simulation, an external pointer returned by
   * the newSimulation function.
   * 
   * @param time the time points to return the logger values.
   * 
   * @return a list of numeric vectors, with time and values reported
   * by all logger.
   * 
   * @details the returned list can be coerced into a data.frame object
   * which first column is time, and other columns are logger results, 
   * each row corresponds to a time point. 
   * 
   * The Simulation object first collect and log the states from all 
   * agents in the simulation, then set the current time to the time of 
   * the first event, then call the resume method to actually run it.
   */
  SEXP runSimulation(SEXP sim, SEXP time);
  /**
   * Continue a previously run simulation
   * 
   * @param sim the simulation, an external pointer returned by
   * the newSimulation function.
   * 
   * @param time the time points to return the logger values.
   * 
   * @return a list of numeric vectors, with time and values reported
   * by all logger.
   * 
   * @details the returned list can be coerced into a data.frame object
   * which first column is time, and other columns are logger results, 
   * each row corresponds to a time point. 
   * 
   * The Simulation object repetitively handle the events until the the 
   * last time point in "time" is reached. ASt each time point, the 
   * logger states are collected in put in a list to return.
   */
  SEXP resumeSimulation(SEXP sim, SEXP time);
  
  /**
   * Add a logger to a simulation
   * 
   * @param sim the simulation, an external pointer returned by
   * the newSimulation function.
   * 
   * @param logger, an external pointer returned by functions like 
   * newCounter or newStateLogger.
   * 
   * @return the exactly same value as sim.
   * 
   * @details without adding a logger, there will be no useful simulation
   * results returned.
   */
  SEXP addLogger(SEXP sim, SEXP logger);

  /**
   * Add a transition to a simulation
   * 
   * @param sim the simulation, an external pointer returned by
   * the newSimulation function.
   * 
   * @param from the starting state for the agent initiating 
   * the transitions.
   * 
   * @param contact_from NULL for a spontaneous transition, or,
   * for the contact of a transition caused by a contact, the 
   * starting state of the contact.
   * 
   * @param to the target state of the transition for the agent.
   * 
   * @param contact_to NULL for a spontaneous transition, or,
   * for the contact of a transition caused by a contact, the 
   * targeting state of the contact.
   * 
   * @param contact NULL for a spontaneous transition, or an external 
   * pointer pointing to a Contact object created by functions such 
   * as newRandomMixing.
   * 
   * @param waiting_time the waiting_time for the transition to occur,
   * a shared_ptr<WaitingTime> point to a WaitingTime object.
   * 
   * @param to_change_callback the R callback function to determine if 
   * the change should occur. See the details section.
   * 
   * @param changed_callback the R callback function after the change
   * happened. See the details section.
   * 
   * @details the transition specifies a spontaneous state change 
   * (i.e., not caused by a contact) from the state "from" to "to".
   * The state of the agent should match "from", and the to_change_callback
   * should be either R_NilValue or return true in order for this 
   * transition to occur. 
   * 
   * For a spontaneous transition, the callback functions take the
   * following two arguments
   *   1. time: the current time in the simulation
   *   2. agent: the agent who initiate the contact, an external pointer
   *   
   * For a transition caused by a contact, the callback functions take
   * the third argument:
   *   3. contact: the contact agent, an external pointer
   */
  SEXP addTransition(SEXP sim,
                     SEXP from, SEXP contact_from, 
                     SEXP to, SEXP contact_to, SEXP contact,
                     SEXP waiting_time, 
                     SEXP to_change_callback, SEXP changed_callback);
}