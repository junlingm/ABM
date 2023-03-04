#pragma once

#include "Contact.h"

class Network : public Contact {
public:
  /**
   * Constructor with the associated population
   * 
   * @param population the associated population
   */
  Network(Population &population);

  /**
   * Destructor
   */  
  virtual ~Network();

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
  virtual const std::vector<PAgent> &contact(double time, Agent &agent);
  
  /** 
   * Add an agent to the contact pattern
   * 
   * @param agent the agent to add
   */
  virtual void add(const PAgent &agent);
  
  /**
   * Getting ready to return the contacts
   */
  virtual void finalize();

protected:
  /**
   * Build the network connections.
   * 
   * @details This function is called in finalize() to let each subclass
   * build the network.
   */
  virtual void build() = 0;

  /**
   * Grow the network and add the agent
   * 
   * @param agent the agent to be added
   * 
   * @details When the network has been finalized, if an agent is added, then
   * this method is called to grow the network and accommodate the new agent.
   */ 
  virtual void grow(const PAgent &agent) = 0;

  /**
   * Connect two nodes
   * 
   * @param from the index of the starting node 
   * 
   * @param to the index of the ending node
   * 
   * @param from and to are the indices of the agents in the population.
   */
  void connect(int from, int to);
  
  /**
   * Whether the network has been finalized.
   * 
   * @details if finalized, then agents added in with be sent to grow().
   */
  bool _finalized;

  /**
   * The array of neighbors for each node.
   */
  std::vector<std::vector<PAgent> > _neighbors;
};

class ConfigurationModel : public Network {
public:
  ConfigurationModel(Population &population, Rcpp::Function degree_rng);
  
protected:
  /**
   * Build the network connections.
   * 
   * @details This function is called in finalize() to let each subclass
   * build the network.
   */
  virtual void build();
  
  /**
   * Grow the network and add the agent
   * 
   * @param agent the agent to be added
   * 
   * @details When the network has been finalized, if an agent is added, then
   * this method is called to grow the network and accommodate the new agent.
   */  
  virtual void grow(const PAgent &agent);
  
  Rcpp::Function _rng;
}; 

extern "C" {
  /**
   * Create a random network Contact object using the configuration models
   * 
   * @param population an external pointer pointing to the population that
   * this Contact object is associated with
   * 
   * @param rng an R function that generates random degrees. 
   * 
   * @details rng takes one argument n, giving the number of degrees to 
   * generate and returns a Rcpp::IntegerVector of length n holding the 
   * generated degrees.
   */
  SEXP newConfigurationModel(SEXP population, SEXP rng);
}