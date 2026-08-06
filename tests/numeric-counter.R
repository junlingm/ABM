library(ABM)

# Named numeric state values work for both occupancy and transition counters.
sim <- Simulation$new(
  2,
  function(i) list(value = if (i == 1) 1.5 else 2.5)
)
agent <- sim$agent(1)
sim$addLogger(newCounter("matching", list(value = 1.5)))
sim$addLogger(newCounter(
  "moves",
  list(value = 1.5),
  list(value = 2.5)
))

initial <- sim$run(0)
stopifnot(
  identical(initial$matching, 1),
  identical(initial$moves, 0)
)

setState(agent, list(value = 2.5))
updated <- sim$resume(1)
stopifnot(
  identical(updated$matching, 0),
  identical(updated$moves, 1)
)

# An atomic numeric argument represents an unnamed, single-domain state.
unnamed_sim <- Simulation$new(1, function(i) 1.5)
unnamed_sim$addLogger(newCounter("matching", 1.5))
unnamed_result <- unnamed_sim$run(0)
stopifnot(identical(unnamed_result$matching, 1))
