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
#' contacts represented by a contact network, and another for social
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
#' specifying the population size, or a list.
#' 
#' @param initializer a function or NULL
#' 
#' @details If population is a number (the population size), then initializer 
#' can be a function that take the index of an agent and return its initial 
#' state.  If it is a list, the length is the population size, and each element
#' corresponds to the initial state of an agent (with the same index).
    initialize = function(population=0, initializer=NULL) {
      if (typeof(population) == "externalptr") {
        super$initialize(population)
        return()
      }
      if (is.list(population)) {
        private$agent = newPopulation(population)
      } else if (is.numeric(population)) {
        private$agent = newPopulation(population, initializer)
      } else stop("invalid population argument")
    },

    #' Add an agent
    #' 
    #' @param agent either an object of the R6 class Agent, or an external
    #' pointer returned from newAgent.
    #' 
    #' @return the population object itself (invisible) for chaining actions
    #' 
    #' @details The agent is scheduled in the population. If the population 
    #' is already added to a simulation, the agent will report its state
    #' to the simulation.
    addAgent = function(agent) {
      if (inherits(agent, "R6Agent"))
        agent = agent$get
      if (!inherits(agent, "Agent"))
        stop("invalid agent argument")
      addAgent(private$agent, agent)
      invisible(self)
    },
    
    #' remove an agent
    #' 
    #' @param agent either an object of the R6 class Agent, or an external
    #' pointer returned from newAgent.
    #' 
    #' @return the population object itself (invisible) for chaining actions
    #' 
    #' @details The agent is scheduled in the population. If the population 
    #' is already added to a simulation, the agent will report its state
    #' to the simulation.
    removeAgent = function(agent) {
      if (inherits(agent, "R6Agent"))
        agent = agent$get
      if (!inherits(agent, "Agent"))
        stop("invalid agent argument")
      removeAgent(private$agent, agent)
      invisible(self)
    },
    
#' Add a contact pattern
#' 
#' @param contact an external pointer pointing to a Contact object,
#' e.g., created from newRandomMixing.
#' 
#' @details If the contact has already been added, this call does nothing.
    addContact = function(contact) {
      if (inherits(contact, "R6Contact"))
        contact = contact$get
      if (!inherits(contact, "Contact"))
        stop("invalid contact argument")
      addContact(private$agent, contact)
      invisible(self)
    },
    
#' return a specific agent by index
#' 
#' @param i the index of the agent (starting from 1)
#' 
#' @return an external pointer pointing to the agent
  agent = function(i) {
      getAgent(private$agent, i)
    },
    
#' set the state of a specific agent by index
#' 
#' @param i the index of the agent (starting from 1)
#' 
#' @param state a list holding the state to set
#' 
#' @return the population object itself (invisible) for chaining actions
    setState = function(i, state) {
      a = getAgent(private$agent, i)
      setState(a, state)
      invisible(self)
    },

#' Set the states for the agents
#' 
#' @param states either a list holding the states (one for each agent), or a
#' function
#' 
#' @return the population object itself for chaining actions
#' 
#' @details If ```states``` is a function then it takes a single argument 
#' ```i```, specifying the index of the agent (starting from 1), and returns
#' a state.
    setStates = function(states) {
      setStates(self$get, states)
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
#' @name newPopulation
#'
#' @param n an integer specifying the population size.
#' 
#' @details The population will be created with "n" individuals in it.
#' These individuals have an empty state upon created. Note that 
#' individuals can be added later by the "add" method, the initial
#' population size is for convenience, not required
#' 
#' @export
NULL

#' Get the size of a population
#' 
#' @name getSize
#' 
#' @param population an external pointer to a population, for example,
#' one returned by [newPopulation()]
#' 
#' @return the population size, an integer
#' 
#' @export
NULL

#' Get the agent at an index in the population
#' 
#' @name getAgent
#' 
#' @param population an external pointer to a population, for example,
#' one returned by [newPopulation()]
#' 
#' @param i the index of the agent, starting from 1.
#' 
#' @return the agent at index i in the population.
#' 
#' @export
NULL

#' Set the state for each agent in a population
#' 
#' @name setStates
#' 
#' @param population an external pointer to a population, for example,
#' one returned by [newPopulation()]
#' 
#' @param states either a list holding the states (one for each agent), or a
#' function
#' 
#' @details If ```states``` is a function then it takes a single argument 
#' ```i```, specifying the index of the agent (starting from 1), and returns
#' a state.
#' 
#' @export
NULL

#' add an agent to a population
#' 
#' @name addAgent
#' 
#' @param population an external pointer to a population, for example,
#' one returned by [newPopulation()]
#' 
#' @param agent an external pointer to an agent, returned by [newAgent()] or 
#' [getAgent()]
#' 
#' @details if the agent is an R6 class, we should use ```agent$get``` to get
#' the external pointer. Similarly, if population is an R6 object, then we
#' should either use ```population$addAgent()``` or ```population$get```.
#' 
#' @export
NULL
