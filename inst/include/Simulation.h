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
   * @param n the initial population size
   * 
   * @param initializer an R function that returns a state for a each agent
   * 
   * @details The simulation object will be created with "n" individuals in it.
   * Note that individuals can be added later by the "add" method, the initial
   * population size is for convenience, not required.
   * 
   * If initializer is R_NilValue, then these individuals have an empty state 
   * upon creation. However, if it is an R function, it must take a single
   * argument "i" giving the iondex or an agent (starting from 1), and return
   * the initial state of the agent.
   */
  Simulation(size_t n = 0, Rcpp::Nullable<Rcpp::Function> initializer = R_NilValue);
  
  /**
   * Constructor 
   * 
   * @param states an R list of initial states, one for each agents
   * 
   * @details The length of the list is the population size, and each element
   * corresponds to the state of the agent at the corresponding index. 
   */
  Simulation(Rcpp::List states);
  
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

  /**
   * Get the next unique ID in the simulation
   * 
   * @return the next ID, an unsigned integer
   * 
   * @details the first ID is 1, and each call to this method will increase by 1.
   */
  unsigned int nextID() { return ++_next_id; }

  /** the simulation that it is in */
  virtual Simulation *simulation();
  /** the simulation that it is in */
  virtual const Simulation *simulation() const;
  

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
  
  /**
   * The next unique ID
   */
  unsigned int _next_id;
};
