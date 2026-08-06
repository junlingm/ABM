library(ABM)

# Resuming through an interval without events must still advance the simulation
# clock. A transition scheduled by a later state change is relative to that
# report time, not the time of the previously handled event.
sim <- Simulation$new(1, function(i) list(state = "inactive"))
agent <- sim$agent(1)
sim$addTransition(
  list(state = "active") -> list(state = "finished"),
  function(time) 5
)
sim$addLogger(newCounter("active", list(state = "active")))
sim$addLogger(newCounter("finished", list(state = "finished")))

invisible(sim$run(0))
invisible(sim$resume(10))
setState(agent, list(state = "active"))
result <- sim$resume(c(14, 16))

stopifnot(
  identical(result$active, c(1, 0)),
  identical(result$finished, c(0, 1))
)
