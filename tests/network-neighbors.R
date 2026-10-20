library(ABM)
expect_error <- function(expr, pattern) {
  message <- tryCatch({force(expr); NULL}, error = conditionMessage)
  stopifnot(!is.null(message), grepl(pattern, message, fixed = TRUE))
}
sim <- Simulation$new(list(list('T'), list('I'), list('S')))
network <- newConfigurationModel(function(n) c(1L, 1L, 0L), rate = 0)
sim$addContact(network)
expect_error(getNeighbors(network, sim$agent(1)), 'not initialized')
invisible(sim$run(0))
set.seed(123)
rng_before <- .Random.seed
neighbors <- getNeighbors(network, sim$agent(1))
stopifnot(length(neighbors) == 1,
          getID(neighbors[[1]]) == getID(sim$agent(2)),
          matchState(neighbors[[1]], 'I'),
          identical(.Random.seed, rng_before),
          length(getNeighbors(network, sim$agent(3))) == 0)
setState(neighbors[[1]], 'T')
stopifnot(matchState(sim$agent(2), 'T'))
expect_error(getNeighbors(newRandomMixing(rate = 0), sim$agent(1)), 'stored network')
other <- Simulation$new(1)
expect_error(getNeighbors(network, other$agent(1)), 'does not belong')

# Stored neighbour references expire on removal, and the adjacency is updated.
removed <- leave(sim$agent(2))
expect_error(getState(neighbors[[1]]), 'expired')
stopifnot(length(getNeighbors(network, sim$agent(1))) == 0)
expect_error(getNeighbors(network, removed), 'does not belong')

# Lookup uses the population-local index, not the simulation-wide agent ID.
nested_sim <- Simulation$new(3)
nested <- Population$new(2)
nested_network <- newConfigurationModel(function(n) rep.int(1L, n), rate = 0)
nested$addContact(nested_network)
nested_sim$addAgent(nested)
invisible(nested_sim$run(0))
stopifnot(getID(getNeighbors(nested_network, nested$agent(1))[[1]]) ==
            getID(nested$agent(2)))

# Population nodes retain their capabilities without extending membership.
population_sim <- Simulation$new()
left <- Population$new(2)
right <- Population$new(3)
population_sim$addAgent(left)
population_sim$addAgent(right)
population_network <- newConfigurationModel(function(n) rep.int(1L, n), rate = 0)
population_sim$addContact(population_network)
invisible(population_sim$run(0))
population_neighbor <- getNeighbors(population_network, left$get)[[1]]
stopifnot(inherits(population_neighbor, 'Population'),
          getSize(population_neighbor) == 3L,
          getID(getAgent(population_neighbor, 1)) == getID(right$agent(1)))
removed_population <- leave(right$get)
stopifnot(getID(removed_population) == right$id,
          getSize(right$get) == 3L,
          length(getNeighbors(population_network, left$get)) == 0L)
expect_error(getSize(population_neighbor), 'expired')
expect_error(getAgent(population_neighbor, 1), 'expired')
