#pragma once

#include "Agent.h"
#include "Contact.h"
#include <vector>
#include <string>
#include <list>
#include <utility>
#include <Rcpp.h>

/**
 * The class that represnts a population
 * 
 * A population is a collection of agents. There are two important tasks
 * for a population:
 *   1. to manage the agents in it
 *   2. to define the contact patterns of the agents
 *   
 * The contact patterns are defined by objects of the Contact class that
 * are associated with the population. A population may have multiple 
 * Contact objects, for example, one for random mixing, one for close
 * contacts represented by a contact netowrk, and another for social
 * network.
 */
class Population : public Agent {
public:
  static constexpr std::uint32_t TAG = Agent::TAG | XP_POPULATION;

  /**
   * Constructor 
   * 
   * @param n the initial population size
   * 
   * @param initializer an R function that returns a state for a each agent
   * 
   * @details The population will be created with "n" individuals in it.
   * Note that individuals can be added later by the "add" method, the initial
   * population size is for convenience, not required.
   * 
   * If initializer is R_NilValue, then these individuals have an empty state 
   * upon creation. However, if it is an R function, it must take a single
   * argument "i" giving the iondex or an agent (starting from 1), and return
   * the initial state of the agent.
   */
  Population(size_t n = 0, Rcpp::Nullable<Rcpp::Function> initializer = R_NilValue);

  /**
   * Constructor 
   * 
   * @param states an R list of initial states, one for each agents
   * 
   * @details The length of the list is the population size, and each element
   * corresponds to the state of the agent at the corresponding index. 
   */
  Population(Rcpp::List states);

  ~Population() override;

  /**
   * Token for handles borrowed for the lifetime of this population.
   */
  const PXPLease &lifetimeLease() const { return _lifetime_lease; }

  /**
   * Add an agent to the populaton
   * 
   * @param agent A shared_ptr<Agent> pointing to the agent to be added
   * 
   * @details The agent is scheduled in the population. If the population 
   * is already added to a simulation, the agent will report its state
   * to the simulation.
   */
  void add(PAgent agent);
  
  /**
   * remove an agent to the populaton
   * 
   * @param agent the agent to be removed
   * 
   * @return returns the agent itself
   * 
   * @details The agent is unscheduled from the population, and all its contact
   * patterns
   */
  PAgent remove(Agent &agent);
  
  /**
   * Add a contact pattern
   * 
   * @param contact A shared_pre<Contact> pointing to a Contact object.
   * 
   * @details If the contact has already been added, this called does 
   * nothing.
   */
  void add(PContact contact);

  /**
   * the population size
   * 
   * It is the number of agents in the population.
   */
  size_t size() const { return _agents.size(); }

  /**
   * return a specific agent by index
   * 
   * @param i the index of the agent (starting from 0)
   * 
   * @return a shared_ptr<Agent> pointing to the agent requested.
   */
  PAgent agentAtIndex(size_t i) const { return _agents[i]; }
  
  /**
   * return a specific agent by ID
   * 
   * @param i the index of the agent (starting from 1)
   * 
   * @return a shared_ptr<Agent> pointing to the agent requested.
   */
  PAgent agent(const Agent &agent) const { return agent._population == this ? _agents[agent._index] : nullptr; }
  
  /**
   * Initialize the state of agents in the population using an 
   * initializer function
   * 
   * @param init an R initializer function. Pleae see details section.
   * 
   * @details The R initializer function should take a single argument
   * specifying the index of the agent to be initialized (starting from 1
   * following R tradition). It should return the initial state of the 
   * agent.
   */
  void initialize(Rcpp::Function init);
  
  /**
   * The classes of an Agent object
   */
  static Rcpp::CharacterVector classes;

  /**
   * reports its state to the simulation.
   * 
   * @details This function should not be called by the user. It is
   * automatically called by the simulation when it needs to collect
   * the state information from the agent.
   */
  void report() override;

protected:
  friend class Simulation;

  /**
   * Expires borrowed R handles when this population is destroyed.
   */
  PXPLease _lifetime_lease;

  /**
   * Assign IDs to this population and all agents contained by it.
   */
  void setID(Simulation &sim) override;

  /**
   * Propagate this population's contacts when it joins an owner population.
   */
  void registered(Population &owner) override;

  /**
   * Remove this population's propagated contacts before it leaves its owner.
   */
  void deregistered(Population &owner) override;

  /**
   * Register or deregister a contact contributed by a descendant population.
   */
  void registerSubcontact(const PContact &contact);
  void deregisterSubcontact(const PContact &contact);
  
  /**
   * A vector holding all agents in the population
   */
  std::vector<PAgent> _agents;
  /**
   * A list of shared_ptr<Contact> pointing to the contacts
   */
  std::list<PContact> _contacts;

  /**
   * Contacts owned by descendant populations.
   */
  std::list<PContact> _subcontacts;
};

typedef std::shared_ptr<Population> PPopulation;
