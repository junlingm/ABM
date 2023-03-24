#pragma once

#include "Event.h"
#include "State.h"
#include <map>

class Population;
class ContactTransition;

/**
 * The class is an abstraction of an agent. The key task of an agent is to
 * maintain events, and handle them in chronological order. Agents also maintain
 * their states, which is a list of values. The events, when handled, operate on
 * the state of the agent (or other agents).
 *
 * During the simulation the agent with the earliest event in the simulation is
 * picked out, unscheduled, then its earliest event is handled, which
 * potentially causes the state change of the agent (or another agent in the
 * simulation). The state change is then logged by loggers that recognize the
 * state change.
 *
 * An agent itself cannot handle the event. Instead, it has to be added to a
 * simulation (or a population that itself is added to a simulation).
 */
class Agent : public Calendar {
public:
  /**
   * Constructor that creates an agent with a given state
   * 
   * @param state an Rcpp List object giving the initial state or R_NilValue 
   * (giving an empty state).
   */
  Agent(Rcpp::Nullable<Rcpp::List> state = R_NilValue);

  /**
   * Returns the agent id (a long value)
   */
  unsigned long id() const { return _id; }

  /**
   * Handle the agent as an event
   * 
   * @param sim the simulationb object
   * 
   * @agent the agent that this event is attached to (scheduled)
   * 
   * @return a boolean value. For an agent, this method always 
   * returns true
   * 
   * @details It first unschedules this agent from the population that 
   * it is in, then unschedules the first (earliest) event, then call 
   * its  handle method. At last, it reschedules itself in the population.
   * 
   * This method should not be called by the user. It is automatically
   * called by the Simulation class.
   */
  virtual bool handle(Simulation &sim, Agent &agent);

  /**
   * Set the state of the agent
   * 
   * @param state an Rcpp List that holds the state
   */
  void set(const Rcpp::List &state);

  /**
   * Check if the state of the agent matches a given rule
   * 
   * @details This is equivalent to this->state().match(state).
   */
  bool match(const Rcpp::List &state) const;
  
  /** 
   * Access the state of the agent
   */
  Rcpp::List state() const { return _state; }

  /**
   * Reports the state to the population the agent is in.
   */
  virtual void report();

  /**
   * remove the agent from the population it was in.
   * 
   * @details after calling this function, the agent is not in any population
   */
  virtual void leave();
  
  /**
   * set the time of death for the agent
   * 
   * @param time the time of death
   * 
   * @details at the given time the agent is removed from the simulation 
   */
  void setDeathTime(double time);

  /** the population that it is in */
  Population *population() { return _population; }
  const Population *population() const { return _population; }

  static Rcpp::CharacterVector classes;

protected:
  /**
   * report to the population that the agent's state has changed
   * 
   * @param agent the agent which state is changing
   * 
   * @param from the original state that the agent was in before 
   * the state change
   */
  virtual void stateChanged(Agent &agent, const State &from);

  Population *_population;

private:
  friend class Population;
  friend class ContactTransition;
  /**
   * The agent id, i.e., the order in the population
   */
  unsigned long _id;
  /**
   * The state of the agent
   */
  State _state;
  /**
   * A calendar holding all the transition events 
   */
  PCalendar _contactEvents;
};
