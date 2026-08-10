library(ABM)

S <- list(stage = "S")
I <- list(stage = "I")

# A transition registered for one contact type ignores contacts of another type.
typed_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
typed_sim$state <- list(S = 1, I = 1)
typed_sim$addContact(newRandomMixing(type = "social"))
typed_sim$addTransition(
  S + I -> I + I ~ "physical",
  function(time) 0
)
typed_sim$addLogger("S")
typed_sim$addLogger("I")
typed_result <- typed_sim$run(c(0, 1))
stopifnot(
  identical(typed_result$S, c(1, 1)),
  identical(typed_result$I, c(1, 1))
)

# Matching physical contact patterns share the same transition rule.
shared_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
shared_sim$state <- list(S = 1, I = 1)
shared_sim$addContact(newRandomMixing(type = "physical"))
shared_sim$addContact(newRandomMixing(type = "physical"))
shared_sim$addTransition(
  S + I -> I + I ~ "physical",
  function(time) 0,
  logging = list(dec("S"), inc("I"))
)
shared_sim$addLogger("S")
shared_sim$addLogger("I")
shared_result <- shared_sim$run(c(0, 1))
stopifnot(
  identical(shared_result$S, c(1, 0)),
  identical(shared_result$I, c(1, 2))
)

# Passing a Contact object remains supported but warns and registers its type.
legacy_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
legacy_sim$state <- list(S = 1, I = 1)
legacy_contact <- newRandomMixing(type = "physical")
legacy_sim$addContact(legacy_contact)
deprecated_warning <- FALSE
withCallingHandlers(
  legacy_sim$addTransition(
    S + I -> I + I ~ legacy_contact,
    function(time) 0,
    logging = list(dec("S"), inc("I"))
  ),
  warning = function(condition) {
    if (grepl("deprecated", conditionMessage(condition), fixed = TRUE))
      deprecated_warning <<- TRUE
    invokeRestart("muffleWarning")
  }
)
stopifnot(deprecated_warning)
legacy_sim$addLogger("S")
legacy_sim$addLogger("I")
legacy_result <- legacy_sim$run(c(0, 1))
stopifnot(
  identical(legacy_result$S, c(1, 0)),
  identical(legacy_result$I, c(1, 2))
)
