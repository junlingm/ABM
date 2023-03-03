#' Creates a RandomMixing object
#' 
#' @param population an external pointer pointing to a population
#' 
#' @return an external pointer.
#' 
#' @details The population must be an external pointer, not an R6 object
#' To use an R6 object, we should use its pointer representation from its
#' $get method.
#' 
#' @export
#' 
#' @examples
#' # creates a simulation with 100 agents
#' sim = Simulation$new(100)
#' # add a random mixing contact pattern for these agents.
#' sim$addContact(newRandomMixing(sim$get))
newRandomMixing = function(population) {
  if (inherits(population, "R6Population"))
    population = population$get
  if (!inherits(population, "Population"))
    stop("invalid population")
  .Call("newRandomMixing", population)
}