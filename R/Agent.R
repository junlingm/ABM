#' R6 class that represent an agent
#'
#' The key task of an agent is to maintain events, and handle them in the
#' chronological order. Agents also maintain their states, which is a list of
#' values. The events, when handled, operate on the state of the agent (or other
#' agents).
#'
#' During the simulation the agent with the earliest event in the simulation is
#' picked out, unscheduled, then its earliest event is handled, which
#' potentially causes the state change of the agent (or another agent in the
#' simulation). The state change is then logged by loggers that recognize the
#' state change.
#'
#' An agent itself cannot handle the event. Instead, it has to be added to a
#' simulation (or a population that itself is added to a simulation).
#' 
#' @export
Agent <- R6::R6Class(
  "R6Agent",
  public = list(
#' Agent 
#' 
#' @param agent can be either an external pointer to an agent such as one
#' returned by newAgent, or a list representing the initial state for creating
#' a new agent, or NULL (an empty state)
#' @param death.time the time of death for the agent, a numeric value
#' 
#' @details Note that specifying death.time is equivalent to call the 
#' ```$setDeathTime``` method.
    initialize = function(agent=NULL, death.time=NA) {
      if (typeof(agent) == "externalptr") {
        private$agent = agent
      } else {
        if (!is.null(agent) && !is.list(agent))
          agent = list(agent)
        private$agent = newAgent(agent, death.time)
      }
    },
    
#' Check if the state of the agent matches a given state
#' 
#' @param rule the state to match, a list
#' 
#' @return a logical value
    match = function(rule) {
      matchState(private$agent, rule)
    },
    
#' Schedule an event
#' 
#' @param event an object of the R6 class Event, or an external pointer 
#' returned by newEvent
#' 
#' @return the agent itself (invisible)
    schedule = function(event) {
      if (inherits(event, "R6Event"))
        event = event$get
      schedule(private$agent, event)
      invisible(self)
    },

#' Unschedule an event
#' 
#' @param event an object of the R6 class Event, or an external pointer 
#' returned by newEvent
#' 
#' @return the agent itself (invisible)
    unschedule = function(event) {
      if (inherits(event, "R6Event"))
        event = event$get
      unschedule(private$agent, event)
      invisible(self)
    },

#' leave the population that the agent is in
#' 
#' @return the agent itself
    leave = function() {
      leave(private$agent)
      invisible(self)
    },

#' set the time of death for the agent
#' 
#' @param time the time of death, a numeric value
#' 
#' @return the agent itself (invisible)
#' 
#' @details At the time of death, the agent is removed from the simulation. 
#' Calling it multiple times causes the agent to die at the earliest time.
    setDeathTime = function(time) {
      setDeathTime(private$agent, time)
      invisible(self)
    }
  ),
  private = list(
    agent = NULL
  ),
  active = list(
#' @field state
#' 
#' Get/set the state of the agent
    state = function(new.state = NULL) {
      if (is.null(new.state)) {
        getState(private$agent)
      } else {
        setState(private$agent, new.state)
      }
    },
#' @field id
#' 
#' Get the agent ID
    id = function() { getID(private$agent) },
#' @field get 
#' 
#' Get the external pointer for the agent
    get = function() { private$agent }
  ),
)

#' Create an agent with a given state
#' 
#' @name newAgent
#' 
#' @param state a list giving the initial state of the agent, or NULL (an empty 
#' list)
#' 
#' @param death_time the death time for the agent, an optional numeric value.
#' 
#' @return an external pointer pointing to the agent
#' 
#' @details Setting death_time is equivalent to calling the [setDeathTime()]
#' function.
#' 
#' @export
NULL

#' Get the ID of the agent.
#' 
#' @name getID
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @return an integer value
#' 
#' @details Before an agent is added to a population, its id is 0.
#' After it is added, its id is the index in the population 
#' (starting from 1).
#' 
#' If agent is an R6 object, then we should either use ```agent$id```, 
#' or use ```getID(agent$get)```
#' 
#' @export
NULL

#' Get the state of the agent
#' 
#' @name getState
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @return a list holding the state
#' 
#' @details If agent is an R6 object, then we should either use agent$schedule, 
#' or use schedule(agent$get, event)
#' 
#' @export
NULL

#' Set the state of the agent
#' 
#' @name setState
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @param state an R list giving the components of the state to be
#' undated.
#' 
#' @details In this framework, a state is a list, each named
#' component is called a domain. This function only updates the 
#' values of the domain given in the "value" list, while leave the
#' other components not in the "value" list unchanged.
#' 
#' If agent is an R6 object, then we should either use agent$schedule, 
#' or use schedule(agent$get, event)
#' 
#' @export
NULL

#' Check if two states match
#' 
#' @name stateMatch
#' 
#' @param state a list holding a state to check
#' 
#' @param rule a list holding the state to match against
#' 
#' @return a logical value
#'
#' @details The state matches the rule if and only if each domain (names of the
#' list) in rule has the same value as in state. The domains in domains of the 
#' state not listed in rule are not matched
#' 
#' @export
NULL

#' Check if the state of an agent matches a given state
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @param rule a list holding the state to match against
#' 
#' @return a logical value
#'
#' @details This function is equivalent to
#' stateMatch(getState(agent), rule)
#' 
#' The state matches the rule if and only if each domain (names of the
#' list) in rule has the same value as in state. The domains in domains of the 
#' state not listed in rule are not matched
#' 
#' @export
matchState = function(agent, rule) {
  stateMatch(getState(agent), rule)
}

#' Schedule (attach) an event to an agent
#' 
#' @name schedule
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @param event an external pointer returned by newEvent
#' 
#' @details If agent is an R6 object, then we should use either 
#' agent$schedule(event) or schedule(agent$get, event)
#' 
#' Similarly, if event is an R6 object, then we should use 
#' schedule(agent, event$get)
#' 
#' @export
NULL

#' Unschedule (detach) an event from an agent
#' 
#' @name unschedule
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @param event an external pointer returned by newEvent
#' 
#' @details If agent is an R6 object, then we should use either 
#' agent$unschedule(event) or unschedule(agent$get, event)
#' 
#' Similarly, if event is an R6 object, then we should use 
#' unschedule(agent, event$get)
#' 
#' @export
NULL

#' Unschedule all event from an agent
#' 
#' @name clearEvents
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @details If agent is an R6 object, then we should use either
#' agent$clearEvents() or clearEvents(agent$get)
#' 
#' @export
NULL

#' leave the population that the agent is in
#' 
#' @name leave
#' 
#' @param agent an external pointer returned by newAgent
#' 
#' @details If agent is an R6 object, then we should use either
#' agent$leave() or leave(agent$get)
#' 
#' @export
NULL

#' set the time of death for an agent
#' 
#' @name setDeathTime
#' 
#' @param agent an external pointer returned by [newAgent()] or [getAgent()]
#' 
#' @param time the time of death, a numeric value
#' 
#' @details If agent is an R6 object, then we should use either
#' agent$leave() or leave(agent$get)
#' 
#' At the time of death, the agent is removed from the simulation. Calling it 
#' multiple times causes the agent to die at the earliest time.
#' 
#' @export
NULL
