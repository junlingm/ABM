#pragma once

#include "Population.h"
#include "Counter.h"
#include "Transition.h"
#include <list>
#include <map>

class Simulation : public Population {
public:
  /**
   * Constructor
   * 
   * @param n the initial population size, a nonnegative integer or
   * NULL (meaning 0)
   * 
   * @details if n is not 0, then n agents initially population
   * the simulation with empty state.
   */
  Simulation(size_t n = 0);
  
  /**
   * Destructor
   */
  virtual ~Simulation();

  /**
   * Run the simulation
   * 
   * @param time the time points to return the logger values.
   * 
   * @return a list of numeric vectors, with time and values reported
   * by all logger.
   * 
   * @details See more details in the resume method.
   */
  virtual Rcpp::List run(const Rcpp::NumericVector &time);

  /**
   * Continue a previously run simulation
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
  virtual Rcpp::List resume(const Rcpp::NumericVector &time);
  
  /**
   * Add a logger to a simulation
   * 
   * @param logger the Logger object to add
   * 
   * @details without adding a logger, there will be no useful simulation
   * results returned.
   */
  void add(std::shared_ptr<Logger> counter);

  /**
   * Add a Transition rule a simulation
   * 
   * @param rule the Transition rule to add
   */
  void add(Transition *rule);
  
  using Population::add;
  
  static Rcpp::CharacterVector classes;
  
protected:
  /**
   * The state of an agent has changed
   * 
   * @param agent the agent whose state has changed
   * 
   * @param from the state of the agent before the change.
   */
  virtual void stateChanged(Agent &agent, const State &from);
  
  std::list<std::shared_ptr<Logger> > _loggers;
  std::list<Transition*> _rules;
  double _current_time;
};
