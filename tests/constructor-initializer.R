library(ABM)

initializer <- function(i) list(index = i)
indexes <- function(population) {
  vapply(
    seq_len(population$size),
    function(i) getState(population$agent(i))$index,
    numeric(1)
  )
}

population <- Population$new(3, initializer)
stopifnot(identical(indexes(population), c(1, 2, 3)))

states <- list(
  list(index = 4, status = "susceptible"),
  list(index = 5, status = "infectious"),
  list(index = 6, status = "recovered")
)
population_from_states <- Population$new(states)
stopifnot(
  identical(population_from_states$size, 3L),
  identical(indexes(population_from_states), c(4, 5, 6)),
  identical(
    vapply(
      seq_len(population_from_states$size),
      function(i) getState(population_from_states$agent(i))$status,
      character(1)
    ),
    c("susceptible", "infectious", "recovered")
  )
)

simulation <- Simulation$new(3, initializer)
stopifnot(identical(indexes(simulation), c(1, 2, 3)))

simulation_from_states <- Simulation$new(states)
stopifnot(
  identical(simulation_from_states$size, 3L),
  identical(indexes(simulation_from_states), c(4, 5, 6)),
  identical(
    vapply(
      seq_len(simulation_from_states$size),
      function(i) getState(simulation_from_states$agent(i))$status,
      character(1)
    ),
    c("susceptible", "infectious", "recovered")
  )
)
