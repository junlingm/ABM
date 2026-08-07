#' Create an event logger that increments a simulation state variable
#'
#' @param variable the name of a numeric state variable in the simulation
#' @param filter an optional function receiving the state of the event agent
#'
#' @return an event logger that can be passed in the `logging` argument of
#'   [Simulation]$addTransition().
#'
#' @export
inc <- function(variable, filter = NULL) {
  newIncrementLogger(variable, filter)
}

#' Create an event logger that decrements a simulation state variable
#'
#' @inheritParams inc
#' @export
dec <- function(variable, filter = NULL) {
  newDecrementLogger(variable, filter)
}
