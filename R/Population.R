#' R6 class that represents a population
#' 
#' A population is a collection of agents. There are two important tasks
#' for a population:
#'   1. to manage the agents in it
#'   2. to define the contact patterns of the agents
#'   
#' The contact patterns are defined by objects of the Contact class that
#' are associated with the population. A population may have multiple 
#' Contact objects, for example, one for random mixing, one for close
#' contacts represented by a contact netowrk, and another for social
#' network.
#' 
#' @export
Population <- R6::R6Class(
  "R6Population",
  inherit = Agent,
  public = list(
#' Population
#'
#' @param population can be either an external pointer pointing to 
#' a population object returned from newPopulation, or an integer 
#' specifying the population size.
#' 
#' @details The population will be created with "n" individuals in it.
#' These individuals have an empty state upon created. Note that 
#' individuals can be added later by the "add" method, the initial
#' population size is for convenience, not required
    initialize = function(population=0) {
      if (typeof(population) == "externalptr") {
        super$initialize(population)
        return()
      }
      t = typeof(population)
      if (t != "double" && t != "integer")
        stop("invalid population argument")
      private$agent = .Call("newPopulation", population, initializer)
      private$managed = TRUE
    },

#' Add an agent
#' 
#' @param agent either an object of the R6 class Agent, or an external
#' pointer returned from newAgent.
#' 
#' @return the object itself for chaining actions.
#' 
#' @details The agent is scheduled in the population. If the population 
#' is already added to a simulation, the agent will report its state
#' to the simulation.
    addAgent = function(agent) {
      method = if (private$managed) "addAgent" else "x_addAgent"
      if (inherits(agent, "R6Agent"))
        agent = agent$get
      if (!inherits(agent, "Agent"))
        stop("invalid agent argument")
      .Call(method, private$agent, agent)
      self
    },
    
#' Add a contact pattern
#' 
#' @param contact an external pointer pointing to a Contact object,
#' e.g., created from newRandomMixing.
#' 
#' @details If the contact has already been added, this called does 
#' nothing.
    addContact = function(contact) {
      if (inherits(contact, "R6Contact"))
        contact = contact$get
      if (!inherits(contact, "Contact"))
        stop("invalid contact argument")
      .Call("addContact", private$agent, contact)
      self
    },
    
#' return a specific agent by index
#' 
#' @param i the index of the agent (starting from 1)
#' 
#' @return an external pointer pointing to the agent
  agent = function(i) {
      .Call("getAgent", private$agent, i)
    },
    
#' set the state of a specific agent by index
#' 
#' @param i the index of the agent (starting from 1)
#' 
#' @param state a list holding the state to set
#' 
#' @return an external pointer pointing to the agent
    setState = function(i, state) {
      a = .Call("getAgent", private$agent, i)
      .Call("setState", a, state)
      self
    }
  ),

  active = list(
#' @field size
#' 
#' The population size, an integer
    size = function() { getSize(private$agent) }
  )
)

#' Create a new population
#'
#' @param n an integer specifying the population size.
#' 
#' @details The population will be created with "n" individuals in it.
#' These individuals have an empty state upon created. Note that 
#' individuals can be added later by the "add" method, the initial
#' population size is for convenience, not required
#' 
#' @export
newPopulation = function(n=0) {
  .Call("newPopulation", n)
}

#' Get the size of a population
#' 
#' @param population an external pointer to a population, for example,
#' one returned by newPopulaton
#' 
#' @return the population size, an integer
#' 
#' @export
getSize = function(population) {
  .Call("getSize", population)
}
