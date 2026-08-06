library(ABM)

zero_degree <- function(n) integer(n)

# Removing before build must not access the empty adjacency array.
unbuilt <- Population$new(3)
unbuilt$addContact(newConfigurationModel(zero_degree))
invisible(leave(unbuilt$agent(3)))
stopifnot(unbuilt$size == 2)

# A network nested in a simulation has simulation-wide IDs that differ from
# its population-local indexes.
sim <- Simulation$new()
nested <- Population$new(4, function(i) list(label = i))
nested$addContact(newConfigurationModel(zero_degree))
sim$addAgent(nested)
invisible(sim$run(0))

invisible(leave(nested$agent(2)))
labels <- vapply(
  seq_len(nested$size),
  function(i) getState(nested$agent(i))$label,
  numeric(1)
)
stopifnot(identical(labels, c(1, 4, 3)))

nested$addAgent(Agent$new(list(label = 5)))
invisible(leave(nested$agent(2)))
stopifnot(nested$size == 3)

# Exercise reciprocal-edge cleanup and adjacency relocation in a connected
# network, followed by growth and another removal from the relocated slot.
set.seed(1)
connected_sim <- Simulation$new()
connected <- Population$new(6, function(i) list(label = i))
connected$addContact(newConfigurationModel(function(n) rep.int(2L, n)))
connected_sim$addAgent(connected)
invisible(connected_sim$run(0))

invisible(leave(connected$agent(3)))
connected$addAgent(Agent$new(list(label = 7)))
invisible(leave(connected$agent(3)))
stopifnot(connected$size == 5)
