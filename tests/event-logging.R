library(ABM)

# Event loggers update numeric state variables only after a transition has
# succeeded. The filter is evaluated on the state of the event agent.
sim <- Simulation$new(
  2,
  function(i) list(stage = "I", age = if (i == 1) 25 else 10)
)
sim$state <- list(I = 2, R = 0, eligible = 0)

I <- list(stage = "I")
R <- list(stage = "R")
sim$addTransition(
  I -> R,
  function(time) 0,
  logging = list(
    dec("I"),
    inc("R"),
    inc("eligible", filter = function(state) state[["age"]] >= 20)
  )
)
sim$addLogger(newStateLogger("I", sim$get, "I"))
sim$addLogger(newStateLogger("R", sim$get, "R"))
sim$addLogger(newStateLogger("eligible", sim$get, "eligible"))

result <- sim$run(c(0, 1))
stopifnot(
  identical(result$I, c(2, 0)),
  identical(result$R, c(0, 2)),
  identical(result$eligible, c(0, 1))
)

# Contact transitions invoke the same event loggers after both sides of the
# contact have been updated.
contact_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
contact_sim$state <- list(S = 1, I = 1)
S <- list(stage = "S")
mixing <- newRandomMixing()
contact_sim$addContact(mixing)
contact_sim$addTransition(
  S + I -> I + I ~ mixing,
  function(time) 0,
  logging = list(dec("S"), inc("I"))
)
contact_sim$addLogger(newStateLogger("S", contact_sim$get, "S"))
contact_sim$addLogger(newStateLogger("I", contact_sim$get, "I"))
contact_result <- contact_sim$run(c(0, 1))
stopifnot(
  identical(contact_result$S, c(1, 0)),
  identical(contact_result$I, c(1, 2))
)

# NULL is the default and leaves the transition without event loggers.
null_logging_sim <- Simulation$new(1, function(i) list(stage = "I"))
null_logging_sim$state <- list(I = 1, R = 0)
null_logging_sim$addTransition(I -> R, function(time) 0)
null_logging_sim$addLogger(newStateLogger("R", null_logging_sim$get, "R"))
null_result <- null_logging_sim$run(c(0, 1))
stopifnot(identical(null_result$R, c(0, 0)))
