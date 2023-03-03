#pragma once

#include <Rcpp.h>
#include <string>

/**
 * Represents the state of an agent as an R list
 */
class State : public Rcpp::List {
public:
  /**
   * Constructor for an empty state
   */
  State();
  
  /**
   * Constructor for a given list as state
   * 
   * @param values an R list that holds the values of the state
   */
  State(Rcpp::List values);
  
  /**
   * Constructor for a state that holds a single value
   * 
   * @param domain the name of the value, can be "" corresponding
   * to a value with a default domain with no name. 
   * 
   * @param value an R value for the domain of the state
   * 
   * @details A domain can have no name (""), which corresponds to the
   * first element in an R list with no name.
   */
  State(const std::string domain, SEXP value);
  
  /**
   * Constructor for a state that holds a single chacacter value
   * with a given list
   * 
   * @param domain the name of the value
   * 
   * @param value a string value
   */
  State(const std::string domain, std::string value);

  /**
   * Constructor for a state that holds a single numeric value
   * 
   * @param domain the name of the value
   * 
   * @param value a numeric value
   */
  State(const std::string domain, double value);
  
  /**
   * returns merged state with a givven list
   * 
   * @param state an R list that holds the domains and their values to
   * be set.
   * 
   * @return a combined state
   * 
   * @details the values in the returned state are replaced by those 
   * in the list for the same domains.
   */
  State operator&(const Rcpp::List &state) const;
  
  /**
   * Set values of domains given by list
   * 
   * @param state an R list that holds the domains and their values to
   * be set.
   * 
   * @return this state
   * 
   * @details the values in this state are replaced by those in the list
   * for the same domains.
   */
  State &operator&=(const Rcpp::List &state);
  
  /**
   * checks if the state contains identical values in the given list
   * for corresponding domains
   * 
   * @param rule the list of domains and their values to match
   * 
   * @return true if the values match, false if at least one value 
   * does not match
   * 
   * @details the values must be either character or numeric vectors.
   */
  bool match(const Rcpp::List &rule) const;
};

extern "C" {
  /**
   * checks if the state contains identical values in the given list
   * for corresponding domains
   * 
   * @param rule an R list holding domains and their values to match
   * 
   * @return a logical value. True if the values match, false if at least one value 
   * does not match
   * 
   * @details the values must be either character or numeric vectors.
   */
  SEXP stateMatch(SEXP state, SEXP rule);
}
