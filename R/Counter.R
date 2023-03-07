#' Create a logger of the Counter class
#' 
#' When state changes occur, it is passed to each logger, which then
#' change its value. At the specified time points in a run, the
#' values of the logger are reported and recorded in a data.frame object,
#' where the columns represent variables, and rows represent the 
#' observation at each time point given to each run. Each logger has a 
#' name, which becomes the the column name in the data.frame.
#' 
#' @name newCounter
#'
#' @param name the name of the counter, must be a length-1 character vector
#'
#' @param from a list specifying state of the agent, or a character or numeric
#'   value that is equivalent to list(from). please see the details section
#'
#' @param to a list (can be NULL) specifying the state of the agent after the
#'   state change, or a character or numeric value that is equivalent to
#'   list(from). please see the details section
#'
#' @param initial the initial value of the counter. Default to 0.
#'
#' @return an external pointer that can be passed to the [Simulation] class' 
#' ```$addLogger```.
#'
#' @details if the argument "to" is not NULL, then the counter counts the
#'   transitions from "from" to "to". Otherwise, it counts the number of agents
#'   in a state that matches the "from" argument. Specifically, if the agent
#'   jumps to "from", then the count increases by 1. If the agents jumps away
#'   from "from", then the count decreases by 1.
#' 
#' @export
NULL

#' Create a logger of the StateLogger class
#' 
#' When state changes occur, it is passed to each logger, which then
#' change its value. At the specified time points in a run, the
#' values of the logger are reported and recorded in a data.frame object,
#' where the columns represent variables, and rows represent the 
#' observation at each time point given to each run. Each logger has a 
#' name, which becomes the the column name in the data.frame.
#' 
#' @name newStateLogger
#'
#' @param name the name of the logger. A length-1 character vector
#' 
#' @param agent the agent whose state will be logged. An external pointer
#' 
#' @param state.name the state name of the state of the agent to be logged.
#' A character vector of length 1.
#' 
#' @details If a state changed happened to any agent, the specified state 
#' of the agent given by the "agent" argument will be logged. If 
#' ```state.name==NULL``` then the state of the agent who just changed is 
#' logged.
#' 
#' The agent must be an external pointer. To use an R6 object, we need
#' to use its $get method to get the external pointer.
#' 
#' The state to be logged must have a numeric value.
#' 
#' @export
NULL
