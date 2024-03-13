#pragma once

#include "Agent.h"
#include "RNG.h"
#include <string>
#include <vector>

class Population;

/**
 * An abstract class that represent the contact pattern of a population. 
 * The main task of the class is to return the contacts of a given agent. 
 * Each contact is associated to a population. A population may have 
 * multiple contacts attached, e.g., a random mixing contact pattern and 
 * a network contact pattern.
 */
class Contact {
public:
  /**
   * Constructor
   */
  Contact();
  
  /**
   * Destructor
   */
  virtual ~Contact();
  
  /**
   * Return the contacts of an agent at a given time
   * 
   * @param time the current time for requesting the contacts
   * 
   * @param agent the agent that requests the contacts
   * 
   * @return a vector of shared_ptr<Agent> that holds the contacts
   */
  virtual const std::vector<Agent*> &contact(double time, Agent &agent) = 0;
  Population *population() { return _population; }

  /**
   * remove an agent
   * 
   * @param agent the agent to be removed
   * 
   * @details this method is called by the population when an agent is removed
   * from it.
   */
  virtual void remove(Agent &agent) = 0;

  /** 
   * Add an agent to the contact pattern
   * 
   * @param the agent that will be added to the contact pattern
   * 
   * @details When an agent is added to a population, it is added to each of the
   * contact patterns. When a contact pattern is added to a population, all
   * agents in a population is added to the contact pattern, one by one. 
   * 
   * Note that, immediately before the simulation is run, while reporting
   * the states to the simulation object, the population will call the 
   * finalize method for each Contact object. Thus a contact object may choose 
   * to ignore addin agents before finalize is called, and handle all agents
   * within the finalize method. However, the contact object must handle
   * adding an agent after finalize is called.
   */
  virtual void add(Agent &agent) = 0;

  /** 
   * Finalize the contact pattern
   * 
   * @param population the associated population
   * 
   * @details This method is called immediately before the simulation is run, 
   * when the attached population reports the states to the simulation object.
   * 
   * Thus this method can be considered as a callback function to notify the 
   * contact object the population state, such as its agents, states, events,
   * and contact patterns are all initialized, so the contact pattern should
   * finish initialization, for example, building the contact network. 
   * 
   * This is needed because some contact patterns, such as a configuration-
   * model contact network, cannot be built while adding agents one by one.
   * It must be generated when all agents are present. This is unlike the 
   * Albert-Barabasi networkm which can be built while adding the agents.
   * 
   * This method set the _population field, then calls the build method
   */
  void attach(Population &population);

  /** 
   * Finalize the contact pattern
   * 
   * @details This method is called from the attach method when the contact
   * pattern is attached to a population. The subclasses must implement this
   * method to build the contact pattern, e.g., the contact network.
   */
  virtual void build() = 0;

  static Rcpp::CharacterVector classes;
  
protected:
  /**
   * The associated population
   */
  Population *_population;
};

typedef std::shared_ptr<Contact> PContact;

/**
 * The random mixing contacct pattern
 */
class RandomMixing : public Contact {
public:
  /**
   * Constructor with the associated population
   * 
   * @param population the associated population
   */
  RandomMixing();

  /**
   * Return the contacts of an agent at a given time
   * 
   * @details A random agent in the population (other than the requesting
   * agent) is returned.
   * 
   * @param time the current time for requesting the contacts
   * 
   * @param agent the agent that requests the contacts
   * 
   * @return a vector of shared_ptr<Agent> that holds the contacts
   */
  virtual const std::vector<Agent*> &contact(double time, Agent &agent);
  
  virtual void add(Agent &agent);
  
  virtual void build();
  
  /**
   * remove an agent
   * 
   * @param agent the agent to be removed
   */
  virtual void remove(Agent &agent);
  
private:
  /**
   * A vector of length one that holds the random contact. This avoids
   * creating the vector and copying the contact
   */
  std::vector<Agent*> _neighbors;
  RUnif _unif;
};

/**
 * An interface to an R6 Contact object
 */
class RContact : public Contact {
public:
  /**
   * Constructor with the associated population and an R6 object
   * 
   * @param population the associated population
   * 
   * @param r6 the R6 object representing a contact pattern
   */
  RContact(Rcpp::Environment r6);
  
  /**
   * Return the contacts of an agent at a given time
   * 
   * @details A random agent in the population (other than the requesting
   * agent) is returned.
   * 
   * @param time the current time for requesting the contacts
   * 
   * @param agent the agent that requests the contacts
   * 
   * @return a vector of shared_ptr<Agent> that holds the contacts
   */
  virtual const std::vector<Agent*> &contact(double time, Agent &agent);
  
  virtual void add(Agent &agent);
  
  virtual void build();
  
  /**
   * remove an agent
   * 
   * @param agent the agent to be removed
   */
  virtual void remove(Agent &agent);
  
private:
  /**
   * A vector of length one that holds the random contact. This avoids
   * creating the vector and copying the contact
   */
  std::vector<Agent*> _neighbors;
  
  /**
   * The R6 object
   */
  Rcpp::Environment _r6;

  /** 
   * The R6 contact method 
   */
  Rcpp::Function _r6_contact;
  
  /** 
   * The R6 addAgent
   */
  Rcpp::Function _r6_addAgent;

  /** 
   * The R6 attach
   */
  Rcpp::Function _r6_attach;

  /** 
   * The R6 remove
   */
  Rcpp::Function _r6_remove;
};
