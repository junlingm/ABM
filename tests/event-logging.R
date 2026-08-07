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
sim$addLogger("I")
sim$addLogger("R")
sim$addLogger("eligible")

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
contact_sim$addLogger("S")
contact_sim$addLogger("I")
contact_result <- contact_sim$run(c(0, 1))
stopifnot(
  identical(contact_result$S, c(1, 0)),
  identical(contact_result$I, c(1, 2))
)

# NULL is the default and leaves the transition without event loggers.
null_logging_sim <- Simulation$new(1, function(i) list(stage = "I"))
null_logging_sim$state <- list(I = 1, R = 0)
null_logging_sim$addTransition(I -> R, function(time) 0)
null_logging_sim$addLogger("R")
null_result <- null_logging_sim$run(c(0, 1))
stopifnot(identical(null_result$R, c(0, 0)))

# A custom output name is supported for state names, while an explicit
# StateLogger retains the name supplied when it was created.
named_sim <- Simulation$new()
named_sim$state <- list(I = 2)
named_sim$addLogger("I", name = "infectious")
state_logger_warning <- FALSE
withCallingHandlers(
  named_sim$addLogger(newStateLogger("explicit", named_sim$get, "I")),
  warning = function(w) {
    if (grepl("StateLogger", conditionMessage(w), fixed = TRUE))
      state_logger_warning <<- TRUE
    invokeRestart("muffleWarning")
  }
)
named_result <- named_sim$run(0)
stopifnot(
  identical(named_result$infectious, 2),
  identical(named_result$explicit, 2),
  state_logger_warning
)

invalid_name <- tryCatch({
  named_sim$addLogger(newStateLogger("I", named_sim$get, "I"), name = "other")
  FALSE
}, error = function(e) TRUE)
stopifnot(invalid_name)
