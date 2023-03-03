#pragma once

#include "Agent.h"
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
   * Constructor with the associated population
   * 
   * @param population the associated population
   */
  Contact(Population &population);
  
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
  virtual const std::vector<PAgent> &contact(double time, Agent &agent) = 0;
  Population &population() { return _population; }

  virtual void add(const Agent *agent) = 0;

  virtual void finalize() = 0;

  static Rcpp::CharacterVector classes;
  
protected:
  /**
   * The associated population
   */
  Population &_population;
};

typedef std::shared_ptr<Contact> PContact;

/**
 * The random mixing contacct pattern
 */
class RandomMixing : public Contact {
public:
  /**
   * Constructor with the sociated population
   * 
   * @param population the associated population
   */
  RandomMixing(Population &population);

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
  
  virtual void add(const Agent *agent);
  
  virtual void finalize();
  
private:
  /**
   * A vector of length one that holds the random contact. This avoids
   * creating the vector and copying the contact
   */
  std::vector<PAgent> _neighbors;
};

extern "C" {
  /**
   * Create an object of the RandomMixing class and wrap it
   * as an external pointer
   * 
   * @param population an external pointer that points to 
   * the associating population.
   */
  SEXP newRandomMixing(SEXP population);
}
