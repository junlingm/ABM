library(ABM)

# A built network must allocate adjacency storage for agents added later.
set.seed(1)
sim <- Simulation$new(4)
network <- newConfigurationModel(function(n) rep.int(2L, n))
sim$addContact(network)
invisible(sim$run(0))
sim$addAgent(Agent$new())
stopifnot(sim$size == 5)

# A zero-degree agent must not index an empty neighborhood sample.
isolated_sim <- Simulation$new(2)
isolated_network <- newConfigurationModel(function(n) rep.int(0L, n))
isolated_sim$addContact(isolated_network)
invisible(isolated_sim$run(0))
isolated_sim$addAgent(Agent$new())
stopifnot(isolated_sim$size == 3)
