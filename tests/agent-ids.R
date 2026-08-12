library(ABM)

agent_ids <- function(population) {
  vapply(
    seq_len(population$size),
    function(i) getID(population$agent(i)),
    integer(1)
  )
}

# Both Simulation constructors start assigning IDs at one.
numeric_sim <- Simulation$new(3)
stopifnot(identical(agent_ids(numeric_sim), 1:3))

state_sim <- Simulation$new(list(list(), list(), list()))
stopifnot(identical(agent_ids(state_sim), 1:3))

# IDs are unique across nested populations and continue for later additions.
sim <- Simulation$new()
first <- Population$new(2)
second <- Population$new(2)
sim$addAgent(first)
sim$addAgent(second)

initial_ids <- c(
  getID(first$get),
  agent_ids(first),
  getID(second$get),
  agent_ids(second)
)
stopifnot(identical(initial_ids, 1:6))

moved <- Agent$new()
first$addAgent(moved)
stopifnot(getID(moved$get) == 7)

second$addAgent(moved)
stopifnot(
  getID(moved$get) == 7,
  first$size == 2,
  second$size == 3
)

# Re-registering a Population assigns IDs to agents added while it was
# detached, even though the Population itself already has an ID.
detached <- Population$new()
sim$addAgent(detached)
detached$leave()
added_while_detached <- Agent$new()
detached$addAgent(added_while_detached)
stopifnot(getID(added_while_detached$get) == 0)
sim$addAgent(detached)
stopifnot(getID(added_while_detached$get) != 0)
