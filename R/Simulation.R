#' R6 class Create and represent a Simulation object
#' 
#' The [Simulation] class inherits the [Population] class. So a simulation
#' manages agents and their contact. Thus, the class also inherits the 
#' [Agent] class. So a simulation can have its own state, and events attached 
#' (scheduled) to it. In addition, it also manages all the transitions, using
#' its ```addTransition``` method. ASt last, it maintains loggers, which
#' record (or count) the state changes, and report their values at specified 
#' times. 
#' 
#' @export
Simulation <- R6::R6Class(
  "R6Simulation",
  inherit = Population,
  public = list(
#' @method initialize
#' Constructor
#'
#' @param simulation can be either an external pointer pointing to 
#' a population object returned from newSimulation, or an integer 
#' specifying the population size, or a list
#' 
#' @param initializer a function or NULL
#' 
#' @details If simulation is a number (the population size), then initializer 
#' can be a function that take the index of an agent and return its initial 
#' state. If it is a list, the length is the population size, and each element
#' corresponds to the initial state of an agent (with the same index).
    initialize = function(simulation = 0, initializer = NULL) {
      if (typeof(simulation) == "externalptr") {
        super$initialize(simulation)
        return()
      }
      if (is.list(simulation)) {
        private$agent = newSimulation(simulation)
      } else if (is.numeric(simulation)) {
        private$agent = newSimulation(simulation, initializer)
      } else stop("invalid simulation argument")
    },
    
#' Run the simulation
#' 
#' @param time the time points to return the logger values.
#' 
#' @return a list of numeric vectors, with time and values reported
#' by all logger.
#' 
#' @details the returned list can be coerced into a data.frame object
#' which first column is time, and other columns are logger results, 
#' each row corresponds to a time point. 
#' 
#' The Simulation object first collect and log the states from all 
#' agents in the simulation, then set the current time to the time of 
#' the first event, then call the resume method to actually run it.
#'
    run = function(time) {
      as.data.frame(runSimulation(self$get, time))
    },
    
#' Continue running the simulation
#' 
#' @param time the time points to return the logger values.
#' 
#' @return a list of numeric vectors, with time and values reported
#' by all logger.
#' 
#' @details the returned list can be coerced into a data.frame object
#' which first column is time, and other columns are logger results, 
#' each row corresponds to a time point. 
#' 
#' The Simulation object repetitively handle the events until the the 
#' last time point in "time" is reached. ASt each time point, the 
#' logger states are collected in put in a list to return.
    resume = function(time) {
      as.data.frame(resumeSimulation(self$get, time))
    },

#' Add a logger to the simulation
#' 
#' @param logger, an external pointer returned by functions like 
#' newCounter or newStateLogger.
#' 
#' @return the simulation object itself (invisible)
#' 
#' @details without adding a logger, there will be no useful simulation
#' results returned.
    addLogger = function(logger) {
      addLogger(self$get, logger)
      invisible(self)
    },

#' Add a transition to the simulation
#' 
#' @param rule is a formula that gives the transition rule
#' 
#' @param waiting.time either an external pointer to a WaitingTime object
#' such as one returned by newExpWaitingTime or newGammaWaitingTime, or
#' a function (see the details section)
#'  
#' @param to_change_callback the R callback function to determine if 
#' the change should occur. See the details section.
#' 
#' @param changed_callback the R callback function after the change
#' happened. See the details section.
#' 
#' @return the simulation object itself (invisible)
#' 
#' @details If waiting.time is a function then it should take exactly one 
#' argument time, which is a numeric value holding the current value, and
#' return a single numeric value for the waiting time (i.e., should not add 
#' time).
#' 
#' Formula can be used to specify either a spontaneous 
#' transition change, or a transition caused by a contact.
#' 
#' A spontaneous transition has the form from -> to, where from and
#' to are state specifications. It is either a variable name holding
#' a state (R list) or the list itself. The list can also be specified 
#' by state(...) instead of list(...)
#' 
#' For a spontaneous transition, the callback functions take the
#' following two arguments
#'   1. time: the current time in the simulation
#'   2. agent: the agent who initiate the contact, an external pointer
#'   
#' A transition caused by contact, the formula needs to specify the 
#' states of both the agent who initiate the contact and the contact
#' agent. The two states are connected by a + sign, the one before the
#' + sign is the initiator, and the one after the sign is the contact.
#' The transition must be associated with a Contact object, using 
#' a ~ operator. The Contact object must be specified by a variable name
#' that hold the external pointer to the object (created by e.g., 
#' the newRandomMixing function) For example, suppose S=list("S"),
#' I=list("I"), and m=newRandomMixing(sim), then a possible rule 
#' specifying an infectious agent contacting a susceptible agent causing 
#' it to become exposed can be 
#' specified by
#' 
#' I + S -> I + list("E") ~ m
#' 
#' For a transition caused by a contact, the callback functions take
#' the third argument:
#'   3. contact: the contact agent, an external pointer
  addTransition = function(rule, waiting.time,
                             to_change_callback = NULL,
                             changed_callback = NULL)
    {
      l = private$parse(substitute(rule), parent.frame())
      addTransition(self$get,
            l$from$first, l$from$second, l$to$first, l$to$second, l$contact,
            waiting.time, to_change_callback, changed_callback)
      invisible(self)
  }
  ),
  private = list(
    # parse a state. 
    # A state is either a variable name or a list (could be state(...))
    # Variable values should be looked up in the 
    # environment envir
    parse.state = function(state, envir) {
      if (is.call(state)) {
        op = as.character(state[[1]])
        if (op != "list" && tolower(op) != "state")
          stop("invalid state ", as.character(state))
        return(as.list(state)[-1])
      } 
      if (is.name(state)) {
        if (!exists(state, envir = envir))
          stop("state ", as.character(state), " is not defined")
        v = get(state, envir=envir)
        if (!is.list(v))
          stop("invalid state ", state, " :", v)
        v
      } else if (!is.null(state)) {
        list(state)
      } else stop("state cannot be NULL")
    }, 
    # parse a side of the transition, either a starting state
    # or a target state (or pairs). If it is a pair connected by ", 
    # it must be a transition caused by contact, and thus must be
    # associated with a contact object following a ~.
    parse.side = function(side, envir) {
      if (is.call(side) && as.character(side[[1]]) == "~") {
        if (!exists(side[[3]], envir=envir))
          stop(paste("the contact", as.character(contact), "does not exist"))
        contact = get(side[[3]], envir=envir)
        if (inherits(contact, "R6Contact"))
          contact = contact$get
        side = side[[2]]
      } else contact = NULL
      if (is.call(side)) {
        op = as.character(side[[1]])
        if (op == "+") {
          first = private$parse.state(side[[2]], envir=envir)
          second = private$parse.state(side[[3]], envir=envir)
        } else {
          first = private$parse.state(side[[1]], envir=envir, envir=envir)
          second = NULL
        }
      } else {
        first = private$parse.state(side, envir=envir)
        second = NULL
      }
      list(first = first, second = second, contact = contact)
    },
    # parse a formula that gives the transition rule
    parse = function(formula, envir) {
      if (as.character(formula[[1]]) != "<-")
        return(NULL)
      to = private$parse.side(formula[[2]], envir=envir)
      from = private$parse.side(formula[[3]], envir=envir)
      contact = from$contact
      if (!is.null(contact) && !is.null(to$contact))
        stop("conact can only be specied once")
      if (is.null(contact)) contact = to$contact
      if ((!is.null(from$second) && is.null(to$second)) ||
          (!is.null(from$second) && is.null(to$second)))
        stop("from and to arguments for a contact must both have two states")
      if (is.null(from$second) && is.null(from$second) && !is.null(contact))
        stop("contact is specified for a non-contact transition")
      list(from=from, to=to, contact=contact)
    }
  )
)
