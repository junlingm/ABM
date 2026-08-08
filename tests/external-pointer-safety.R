library(ABM)

# A handle for a base class accepts a derived object.
sim <- Simulation$new()
stopifnot(identical(getID(sim$get), 0L))

# A handle for a derived class rejects an object that only supports the base.
agent <- newAgent(NULL)
wrong_type <- try(ABM:::runSimulation(agent, 0), silent = TRUE)
stopifnot(
  inherits(wrong_type, "try-error"),
  grepl("does not support the requested type", wrong_type, fixed = TRUE)
)

# Event and Agent share a C++ root holder, but their capability tags prevent
# an Event from being interpreted as an Agent.
event <- newEvent(1, function(...) NULL)
wrong_family_member <- try(getID(event), silent = TRUE)
stopifnot(
  inherits(wrong_family_member, "try-error"),
  grepl(
    "does not support the requested type",
    wrong_family_member,
    fixed = TRUE
  )
)

# Callback arguments work during the callback but expire immediately after it.
saved_agent <- NULL
saved_sim <- NULL
sim <- Simulation$new(list(list(value = 1)))
event <- newEvent(0, function(time, callback_sim, callback_agent) {
  stopifnot(identical(getState(callback_agent)$value, 1))
  saved_agent <<- callback_agent
  saved_sim <<- callback_sim
})
schedule(sim$agent(1), event)
invisible(sim$run(c(0, 1)))

expired_agent <- try(getState(saved_agent), silent = TRUE)
expired_sim <- try(getState(saved_sim), silent = TRUE)
stopifnot(
  inherits(expired_agent, "try-error"),
  inherits(expired_sim, "try-error"),
  grepl("callback handle has expired", expired_agent, fixed = TRUE),
  grepl("callback handle has expired", expired_sim, fixed = TRUE)
)

# Transition callbacks use the same scoped-borrow contract.
saved_transition_agent <- NULL
transition_sim <- Simulation$new(1, function(i) list(state = "S"))
transition_sim$addTransition(
  list(state = "S") -> list(state = "I"),
  function(time) 0,
  to_change_callback = function(time, agent) {
    stopifnot(identical(getState(agent)$state, "S"))
    saved_transition_agent <<- agent
    TRUE
  }
)
invisible(transition_sim$run(c(0, 1)))
expired_transition_agent <- try(
  getState(saved_transition_agent),
  silent = TRUE
)
stopifnot(
  inherits(expired_transition_agent, "try-error"),
  grepl(
    "callback handle has expired",
    expired_transition_agent,
    fixed = TRUE
  )
)
