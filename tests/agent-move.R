library(ABM)

# Adding an existing agent transfers membership and preserves its state.
source <- Population$new(1, function(i) list(status = "present"))
destination <- Population$new()
agent <- source$agent(1)
destination$addAgent(agent)

stopifnot(
  source$size == 0,
  destination$size == 1,
  identical(getState(agent)$status, "present")
)

# Moving during an event must transfer calendar ownership as well.
sim <- Simulation$new()
source <- Population$new(
  1,
  function(i) list(status = "present", fired = FALSE)
)
destination <- Population$new()
sim$addAgent(source)
sim$addAgent(destination)
agent <- source$agent(1)

move_agent <- newEvent(1, function(time, sim, event_agent) {
  destination$addAgent(event_agent)
})
future_event <- newEvent(2, function(time, sim, agent) {
  setState(agent, list(fired = TRUE))
})
schedule(agent, move_agent)
schedule(agent, future_event)

sim$addLogger(newCounter("present", list(status = "present")))
result <- sim$run(c(0, 1.5))

stopifnot(
  source$size == 0,
  destination$size == 1,
  identical(result$present, c(1, 1))
)

# Once removed from the destination, the agent's pending event must not be
# reachable through the source population's calendar.
leave(agent)
stopifnot(destination$size == 0)
invisible(sim$resume(3))
stopifnot(identical(getState(agent)$fired, FALSE))
