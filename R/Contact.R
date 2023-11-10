#' An R6 class that implements a contact pattern in R
#'
#' The main task of the class is to return the contacts of a given agent. Each
#' object of this class is associated to a population. A population may have
#' multiple contacts attached, e.g., a random mixing contact pattern and a
#' network contact pattern.
#'
#' @details This class must be subclassed in order to implement specific functionality.
#' To subclass, we must implement three methods, namely contact, addAgent, and
#' build. See more details in the documentation of each method.
#' 
#' @export
Contact = R6::R6Class(
  "R6Contact",
  public = list(
#' @description the constructor
    initialize = function() {
      private$pointer = newContact(self)
    },
  
#' @description attach to a population
#'
#' @param population the population to attach to. An external pointer
#' 
#' @details. This method should be called from the C++ side. Users should not
#' call this directly.
    attach = function(population) {
      if (!is.null(private$population))
        stop("Already attached to a population")
      private$population = population
      self$build()
    },

#' @description Returns the contacts of the given agent
#' 
#' @param time the current time in the simulation, a number
#' 
#' @param agent the agent whose contacts are requested. An external pointer
#' 
#' @return a list of external pointers pointing to the contacting agents
    contact = function(time, agent) { list() },
    
#' @description Add an agent to the contact pattern
#' 
#' @param agent the agent to be added. An external pointer
#' 
#' @details When an agent is added to a population, it is added to each of the
#' contact patterns. When a contact pattern is added to a population, all
#' agents in a population is added to the contact pattern, one by one. 
#' 
#' Note that, immediately before the simulation is run, while reporting
#' the states to the simulation object, the population will call the 
#' build method for each Contact object. Thus a contact object may choose 
#' to ignore adding agents before build is called, and handle all agents
#' within the finalize method. However, the contact object must handle
#' adding an agent after build is called.
    addAgent = function(agent) { },

#' @description Remove an agent from the contact pattern
#' 
#' @param agent the agent to be removed. An external pointer
#' 
#' @details When an agent leaves a population, it is removed from each of the
#' contact patterns.
#' 
#' This method may also be called in event handlers to remove an agent
    remove = function(agent) { },

#' @description Build the contact pattern
#' 
#' @details This method is called immediately before the simulation is run, 
#' when the attached population reports the states to the simulation object.
#' 
#' Thus this method can be considered as a callback function to notify the 
#' contact object the population state, such as its agents, states, events,
#' and contact patterns are all initialized, so the contact pattern should
#' finish initialization, for example, building the contact network. 
#' 
#' This is needed because some contact patterns, such as a configuration-
#' model contact network, cannot be built while adding agents one by one.
#' It must be generated when all agents are present. This is unlike the 
#' Albert-Barabasi networkm which can be built while adding the agents.
    build = function() { }
  ),
  private = list(
    pointer = NULL,
    population = NULL
  ),
  active = list(
#' @field get
#'
#'.The external pointer pointing to the C++ RContact object.
    get = function() { private$pointer },
    
  #' @field attached 
  #' 
  #' a logical value indicating whether the object has been attached
  #' to a population
    attached = function() { !is.null(private$population) }
  )
)

#' Creates a RandomMixing object
#' 
#' @return an external pointer.
#' 
#' @name newRandomMixing
#'
#' @export
#' 
#' @examples
#' # creates a simulation with 100 agents
#' sim = Simulation$new(100)
#' # add a random mixing contact pattern for these agents.
#' sim$addContact(newRandomMixing())
#' 
#' @export
NULL

#' Creates a random network using the configuration model
#'
#' @name newConfigurationModel
#' 
#' @param rng a function that generates random degrees
#'
#' @return an external pointer.
#' 
#' @details The population must be an external pointer, not an R6 object
#' To use an R6 object, we should use its pointer representation from its
#' $get method.
#' 
#' The function rng should take exactly one argument n for the number of degrees
#' to generate, and should return an integer vector of length n.
#'
#' @examples
#' # creates a simulation with 100 agents
#' sim = Simulation$new(100)
#' # add a Poisson network with a mean degree 5
#' sim$addContact(newConfigurationModel(function(n) rpois(n, 5)))
#' 
#' @export
NULL
