#' R6 class to create and represent an event
#' 
#' @examples
#' # This handler prints increases a counter in the state of the 
#' # Simulation object, and schedule another event every 0.1 time unit.
#' handler = function(time, sim, agent) {
#'   x = getState(sim)
#'   x$counter = x$counter + 1
#'   setState(sim, x)
#'   schedule(agent, newEvent(time + 0.1, handler))
#' }
#' # create a new simulation with no agents. but the simulation itself is
#' # an agent. So we can use all the methods of agent
#' sim = Simulation$new()
#' # set the state of the simulation, initialize the counter
#' sim$state = list(counter = 0)
#' # schedule a new event at time 0
#' sim$schedule(Event$new(0, handler))
#' # add a logger for the counter. Note that, because sim is an R6 class
#' # to use it in the newStateLogger function, we need to access the 
#' # external pointer using its $get method
#' sim$addLogger(newStateLogger("counter", sim$get, "counter"))
#' # run the simulation for 10 time units.
#' print(sim$run(0:10))
#' # interestingly, the counts are not exactly in 10 event time unit. 
#' # Firstly, report always happen before event, so event at time 0 is 
#' # not counted in the time interval 0 to 1. Secondly, the event time 
#' # is stored as a numeric value with increments of 0.1, which is 
#' # subject to rounding errors. So some the the integer tiome events
#' # may be before the reporting and some may be after.
#' 
#' @export
Event <- R6::R6Class(
  "R6Event",
  public = list(
    #' Event
    #' 
    #' @param time the time that this event will occur. A length-1
    #' numeric vector.
    #' 
    #' @param handler an R function that handles the event when it occurs.
    #' 
    #' @details The R handler function should take exactly 3 arguments
    #'  1. time: the current time in the simulation
    #'  2. sim: the simulation object, an external pointer
    #'  3. agent: the agent to whom this event is attached to. 
    #'  
    #' The return value of the handler function is ignored.
    initialize = function(time, handler) {
      private$event = newEvent(time, handler)
    }
  ),
  private = list(
    event = NULL
  ),
  active = list(
    #' @field time
    #' 
    #' returns the event time
    #' 
    time = function() {
      getTime(private$event)
    },
    
    #' @field get
    #' 
    #' returns the external pointer, which can then be passed to
    #' functions such as schedule and unschedule.
    get = function() {
      private$event
    }
  )
)

#' Creates a new event in R
#' 
#' @name newEvent
#'
#' @param time the time that this event will occur. A length-1
#' numeric vector.
#' 
#' @param handler an R function that handles the event when it occurs.
#' 
#' @return an external pointer, which can then be passed to
#' functions such as schedule and unschedule.
#' 
#' @details The R handler function should take exactly 3 arguments
#'  1. time: the current time in the simulation
#'  2. sim: the simulation object, an external pointer
#'  3. agent: the agent to whom this event is attached to. 
#'  
#' The return value of the handler function is ignored.
#' 
#' This function avoids the overhead of an R6 class, and is thus faster.
#' This is the recommended method to create an event in an event handler.
#' 
#' @export
NULL

#' returns the event time
#' 
#' @name getTime
#' 
#' @param event an external pointer returned by the newEvent function.
#' 
#' @return a numeric value
#' 
#' This function avoids the overhead of an R6 class, and is thus faster.
#' This is the recommended method to get event time in an event handler.
#' 
#' @export
NULL
