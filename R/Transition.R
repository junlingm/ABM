#' Creates an exponentially distributed waiting time
#' 
#' @name newExpWaitingTime
#' 
#' @param rate the rate of the exponential distribution
#' 
#' @return an external pointer
#' 
#' @details This function creates an C++ object of type ExpWaitingTime. 
#' It can be passed to addTransition or Simulation$addTransition to
#' specify the waiting time for a transition. As a C++ object, it is faster
#' than using an R function to generate waiting times because there is
#' no need to call an R function from C++.
#' 
#' @export
NULL

#' Creates an gamma distributed waiting time
#' 
#' @name newGammaWaitingTime
#' 
#' @param shape the shape parameter of the gamma distribution
#' 
#' @param scale the scale parameter of the gamma distribution, i.e., 1/rate
#' 
#' @return an external pointer
#' 
#' @details This function creates an C++ object of type ExpWaitingTime. 
#' It can be passed to addTransition or Simulation$addTransition to
#' specify the waiting time for a transition. As a C++ object, it is faster
#' than using an R function to generate waiting times because there is
#' no need to call an R function from C++.
#' 
#' @export
NULL

#' Generate a waiting time from an WaitingTime object
#' 
#' @name getWaitingTime
#' 
#' @param generator an external pointer to a WaitingTime object, e.g.,
#' one returned by newExpWaitingTime or newGammaWaitingTime
#' 
#' @param time the current simulation time, a numeric value
#' 
#' @return a numeric value
#' 
#' @export
NULL
