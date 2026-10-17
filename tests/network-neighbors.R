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
