#pragma once

#include "XP.h"
#include <map>

class Agent;
typedef std::shared_ptr<Agent> PAgent;

class Simulation;
typedef std::shared_ptr<Simulation> PSimulation;

class Event;
typedef std::shared_ptr<Event> PEvent;

/**
 * An abstract class that represent an event.
 * 
 * Events are a crucial component of the framework. An event has 
 * a time and a handler, and is attached (scheduled) to an agent. The 
 * events attached to an agent are sorted by their time. The agent itself
 * is an event that represents its earliest event. 
 * 
 * During the simulation the agent with the earliest event in the 
 * simulation is picked out, unscheduled, then its earliest event 
 * is handled, which potentially causes the state change of the 
 * agent (or another agent in the simulation). The state change 
 * is then logged by loggers that recognize the state change.
 */
class Event {
public:
  /**
   * constructor with the event time
   * 
   * @param time the time that the event will happen
   */
  Event(double time);
  /**
   * Destructor
   */
  virtual ~Event();

  /**
   * Returns the event time
   */
  double time() const { return _time; }
  
  /**
   * The agent that this event is attached to (i.e., scheduled)
   */
  const Agent *owner() const { return _owner; }
  
  /**
   * Handle the event
   * 
   * @param sim the simulation object
   * 
   * @agent the agent that this event is attached to (scheduled)
   * 
   * @return a boolean value. If true, the event will be rescheduled 
   * after handling. If false, the event will be discarded after 
   * handling.
   * 
   * @details Returning true means that the handler has revised the 
   * event for reuse. It has two use cases, one is that agent is an 
   * event, which should not be discarded after handling its first event. 
   * Another is to reuse the event to avoid the overhead of creating new 
   * events.
   * 
   * Note that, at the time of event handling, the event has been
   * unscheduled. So the owner of the event is cleared when the handler
   * if executed. This is why we need an "agent" argument.
   * 
   * This method should not be called by the user. It is automatically
   * called by the Simulation class.
   */
  virtual bool handle(Simulation &sim, Agent &agent) = 0;

  /**
   * the class of an Event object.
   */
  static Rcpp::CharacterVector classes;
  
protected:
  /**
   * the agent that this event is attached (scheduled to)
   */
  Agent *_owner;
  /**
   * the time that this event will happen
   */
  double _time;
  
private:
  friend class Agent;
  /**
   * saves the position in the event tree of the attached agent, to 
   * speed up event unschedule.
   */
  std::multimap<double, PEvent>::iterator _pos;
};

/**
 * An event that takes an R function as handler
 */
class REvent : public Event {
public:
  /**
   * Constructor
   * 
   * @param time the time that this event will happen
   * 
   * @param handler An R function that handles the event. 
   * 
   * @details The R handler function should take exactly 3 arguments
   *   1. time: the current time in the simulation
   *   2. sim: the simulation object, an external pointer
   *   3. agent: the agent to whom this event is attached to. 
   *   
   * The return value of the handler function is ignored.
   */
  REvent(double time, Rcpp::Function handler);
  
  /**
   * Handle the event
   * 
   * @param sim the simulation object
   * 
   * @agent the agent that this event is attached to (scheduled)
   * 
   * @return a boolean value. REvent class always returns false to
   * avoid resue the event, because revising event time in R is not
   * possible.
   * 
   * @details This class simply calls the R handler function.
   * 
   * This method should not be called by the user. It is automatically
   * called by the Simulation class.
   */
  virtual bool handle(Simulation &sim, Agent &agent);

protected:
  /**
   * The R handler function.
   */
  Rcpp::Function _handler;
};
