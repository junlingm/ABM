#pragma once

#include "Contact.h"
#include "RNG.h"
#include <string>

/**
 * A class representing a random waiting time for a stqte transition
 */
class WaitingTime {
public:
  /**
   * Destructor
   */
  virtual ~WaitingTime();

  /**
   * generate a random waiting time
   * 
   * @param time the current time
   * 
   * @return the generated waiting time
   * 
   * @details Note that the waiting time is the period between the current
   * time and the time that the state transition occurs, not the time
   * of state transition (which is time + waitingTime(time)). 
   */
  virtual double waitingTime(double time) = 0;

  /**
   * The classes of WaitingTime objects
   */
  static Rcpp::CharacterVector classes;
};

typedef std::shared_ptr<WaitingTime> PWaitingTime;

/**
 * Represents a normal (spontaneous) state transition
 * 
 * The state transitions represented by this class are not caused by 
 * agent contacts (such as disease transmission). They may be, 
 * for example, recovery. 
 */
class Transition {
public:
  /**
   * Constructor
   * 
   * @param from the starting state of the transition. 
   * 
   * @param to the target state of the transition
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
   * Each function takes two arguments
   *   1. time: the current time in the simulation
   *   2. agent: the agent who initiate the contact, an external pointer
   */
  Transition(const Rcpp::List &from, const Rcpp::List &to, 
             PWaitingTime waiting_time, 
             Rcpp::Nullable<Rcpp::Function> to_change_callback=R_NilValue, 
             Rcpp::Nullable<Rcpp::Function> changed_callback=R_NilValue);
  
  /**
   * Destructor
   */
  virtual ~Transition();

  /**
   * returns the state for the agent to be matched before the 
   * transition. The transition will not happen if the state if 
   * does not match.
   */
  const Rcpp::List &from() const { return _from; }
  
  /**
   * returns the state for the agent to be set after the transition
   */
  const Rcpp::List &to() const { return _to; }
  
  /**
   * Calls the R callback function before the state change
   * to determine if the state change should happen
   * 
   * @param time the simulation time
   * 
   * @param agent the agent to initiate the contact
   * 
   * @return true if the state transition should happen; false will 
   * present the transition from happening.
   */
  bool toChange(double time, Agent &agent);

  /**
   * Calls the R callback function after the state change happended.
   * 
   * @param time the simulation time
   * 
   * @param agent the agent to initiate the contact
   */
  void changed(double time, Agent &agent);

  /**
   * Schedule an agent for the next contact event
   * 
   * @param time the currrent simulation time
   * 
   * @param agent the agent to schedule a contact event
   * 
   * @details the agent must match the state in from specified
   * in the constructor
   */
  virtual void schedule(double time, Agent &agent);

  /**
   * The R classes of a Transition object
   */
  static Rcpp::CharacterVector classes;
  
protected:
  /**
   * the state of the agent to be matched before transition. The
   * transition happens only if the state of the agent matches 
   * this state.
   */
  Rcpp::List _from;

  /**
   * the state of the agent to be set to after transition
   */
  Rcpp::List _to;

  PWaitingTime _waiting_time;
  /**
   * The R callback function before the state changes to determine
   * if the state change should happen (if return true) or not (if
   * return false)
   */
  Rcpp::Function *_to_change;
  /**
   * The R callback function after the state has changed
   */
  Rcpp::Function *_changed;
};

/**
 * Represents a state transition caused by contacts
 * 
 * The state transitions represented by this class are caused by 
 * agent contacts (such as disease transmission).
 */
class ContactTransition : public Transition {
public:
  /**
   * Constructor
   * 
   * @param agent_from the starting state of the transition for the 
   * agent initiating the contact.
   * 
   * @param contact_from the starting state of the transition for the 
   * contact. 
   * 
   * @param agent_to the target state of the transition for the agent
   * who initialting the contact
   * 
   * @param contact_to the targeting state for the contact
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
   * Unlike the callbacks for the Transition class, each function takes 
   * three arguments
   *   1. time: the current time in the simulation
   *   2. agent: the agent who initiate the contact, an external pointer
   *   3. contact: the contact agent, an external pointer
   */
  ContactTransition(const Rcpp::List &agent_from, const Rcpp::List &contact_from, 
     const Rcpp::List &agent_to, const Rcpp::List &contact_to,
     Contact &contact,
     PWaitingTime waiting_time, 
     Rcpp::Nullable<Rcpp::Function> to_change_callback=R_NilValue, 
     Rcpp::Nullable<Rcpp::Function> changed_callback=R_NilValue);
  
  /**
   * returns the state for the contact to be matched before the 
   * transition. The contact will not happen if the state of the 
   * contact does not match.
   */
  const Rcpp::List &contactFrom() const { return _contact_from; }
  
  /**
   * returns the state for the contact to be set after the transition
   */
  const Rcpp::List &contactTo() const { return _contact_to; }

  /**
   * returns the contact pattern
   */
  Contact &contact() const { return _contact; }
  
  /**
   * Calls the R callback function before the state change
   * to determine if the state change should happen
   * 
   * @param time the simulation time
   * 
   * @param agent the agent to initiate the contact
   * 
   * @param contact the agent to be contacted
   * 
   * @return true if the contact should happen; false will present the
   * event from happening.
   */
  bool toChange(double time, Agent &agent, Agent &contact);
  
  /**
   * Calls the R callback function after the state change happended.
   * 
   * @param time the simulation time
   * 
   * @param agent the agent to initiate the contact
   * 
   * @param contact the agent to be contacted
   */
  void changed(double time, Agent &agent, Agent &contact);
  
  /**
   * Schedule an agent for the next contact event
   * 
   * @param time the currrent simulation time
   * 
   * @param agent the agent to schedule a contact event
   * 
   * @details the agent must match the state in agent_from specified
   * in the constructor
   */
  virtual void schedule(double time, Agent &agent);
  
protected:
  /**
   * matching the state of the contacted agent before contact
   */
  Rcpp::List _contact_from;

  /**
   * the state of the contacted agent to be set to after contact
   */
  Rcpp::List _contact_to;
  /** 
   * the contact pattern 
   */
  Contact &_contact;
};

/**
 * Generates an exponentially distributed waiting time
 */
class ExpWaitingTime : public WaitingTime {
public:
  /**
   * Constructor for an exponentially distributed waiting time
   * 
   * @param rate the rate of the exponential distribution
   */
  ExpWaitingTime(double rate);
  
  /**
   * generate an exponentially distributed waiting time
   * 
   * @param time the current time
   * 
   * @return the generated waiting time
   * 
   * @details Note that the waiting time is the period between the current
   * time and the time that the state transition occurs, not the time
   * of state transition (which is time + waitingTime(time)). 
   */
  virtual double waitingTime(double time);
  
protected:
  /** 
   * the batched exponential random number generator
   */
  RExp _exp;
};

/**
 * Generate a gamma distributed waiting time
 */
class GammaWaitingTime : public WaitingTime {
public:
  /**
   * Constructor for a gamma distributed waiting time
   * 
   * @param shape the shape parameter of the gamma distribution
   * 
   * @param scale the scale parameter of the gamma distribution 
   * i.e., 1/rate
   */
  GammaWaitingTime(double shape, double scale);
  
  /**
   * generate a gamma distributed waiting time
   * 
   * @param time the current time
   * 
   * @return the generated waiting time
   * 
   * @details Note that the waiting time is the period between the current
   * time and the time that the state transition occurs, not the time
   * of state transition (which is time + waitingTime(time)). 
   */
  virtual double waitingTime(double time);
  
protected:
  /**
   * the shape parameter of the gamma distribution
   */
  double _shape;
  /**
   * the scale parameter of the gamma distribution, i.e., 1/rate
   */
  double _scale;
};

/**
 * Using an R function to generates a waiting time 
 */
class RWaitingTime : public WaitingTime {
public:
  /**
   * Constructor
   * 
   * @param f the R functon to generate a waiting time.
   * 
   * @details The R function should take exactly one argument "time"
   * which is the current simulation time, and should return a 
   * NumericVector of length 1 that holds the random waiting time.
   */
  RWaitingTime(Rcpp::Function f);
  
  /**
   * generate a random waiting time
   * 
   * @param time the current time
   * 
   * @return the generated waiting time
   * 
   * @details Note that the waiting time is the period between the current
   * time and the time that the state transition occurs, not the time
   * of state transition (which is time + waitingTime(time)). 
   */
  virtual double waitingTime(double time);
  
protected:
  /**
   * The R function to generate a random waiting time.
   * 
   * @details Note that the waiting time is the period between the current
   * time and the time that the state transition occurs, not the time
   * of state transition (which is time + waitingTime(time)). 
   */
  Rcpp::Function _f;
};
