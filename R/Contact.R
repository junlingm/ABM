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

#' Creates a random network using the configvuration model
#' 
#' @param population an external pointer pointing to a population
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
#' @export
#' 
#' @examples
#' # creates a simulation with 100 agents
#' sim = Simulation$new(100)
#' # add a Poisson network with a mean degree 5
#' sim$addContact(newConfigurationModel(sim$get, function(n) rpois(n, 5)))
newConfigurationModel = function(population, rng) {
  if (inherits(population, "R6Population"))
    population = population$get
  if (!inherits(population, "Population"))
    stop("invalid population")
  if (!is.function(rng))
    stop("rng must be a function")
  .Call("newConfigurationModel", population, rng)
}
