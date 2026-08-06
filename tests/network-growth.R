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

# Growth samples existing nodes in proportion to their degrees. With this
# seed, both sampled stubs select the second agent. They therefore produce one
# edge after duplicate-edge removal, rather than being assigned to the first
# and second agents based on their positions in the sample.
set.seed(4)
degree_rng <- function(n) {
  if (n == 3) c(0L, 1L, 1L) else rep.int(2L, n)
}
dynamic_sim <- Simulation$new(
  3,
  function(i) list(role = "existing", label = i)
)
dynamic_network <- newConfigurationModel(degree_rng)
dynamic_sim$addContact(dynamic_network)

waiting_calls <- 0L
contacted_label <- NA_integer_
dynamic_sim$addTransition(
  list(role = "new", status = "active") + list(role = "existing") ->
    list(status = "finished") + list(role = "existing") ~ dynamic_network,
  function(time) {
    waiting_calls <<- waiting_calls + 1L
    0
  },
  changed_callback = function(time, agent, contact) {
    contacted_label <<- getState(contact)$label
  }
)
invisible(dynamic_sim$run(0))

new_agent <- Agent$new(list(role = "new", status = "inactive"))
dynamic_sim$addAgent(new_agent)
setState(new_agent$get, list(role = "new", status = "active"))

# Scheduling asks for one waiting time per neighbor.
stopifnot(waiting_calls == 1L)
invisible(dynamic_sim$resume(1))
stopifnot(contacted_label == 2L)
