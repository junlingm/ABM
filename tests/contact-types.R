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

# A contact without a rate emits a migration warning.
unrated_sim <- Simulation$new(1)
unrated_sim$addContact(newRandomMixing(type = "unrated"))
unrated_warning <- FALSE
withCallingHandlers(
  unrated_sim$run(0),
  warning = function(condition) {
    if (grepl("Contact has no rate", conditionMessage(condition), fixed = TRUE))
      unrated_warning <<- TRUE
    invokeRestart("muffleWarning")
  }
)
stopifnot(unrated_warning)

# With one distinct contact type, the type can be inferred by omitting ~.
inferred_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
inferred_sim$state <- list(S = 1, I = 1)
# A positional rate uses the default contact type, so the type can be inferred.
inferred_sim$addContact(newRandomMixing(function(time) 0))
inferred_sim$addTransition(
  S + I -> I + I,
  logging = list(dec("S"), inc("I"))
)
inferred_sim$addLogger("S")
inferred_sim$addLogger("I")
inferred_result <- inferred_sim$run(c(0, 1))
stopifnot(
  identical(inferred_result$S, c(1, 0)),
  identical(inferred_result$I, c(1, 2))
)

# Contact transitions use the direct contacts of the agent's population,
# including for agents in a nested population.
nested_contact_sim <- Simulation$new()
nested_contact_population <- Population$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
nested_contact_population$addContact(newRandomMixing(
  rate = function(time) 0,
  type = "nested"
))
nested_contact_sim$addAgent(nested_contact_population)
nested_contact_sim$state <- list(S = 1, I = 1)
nested_contact_sim$addTransition(
  S + I -> I + I ~ "nested",
  logging = list(dec("S"), inc("I"))
)
nested_contact_sim$addLogger("S")
nested_contact_sim$addLogger("I")
nested_contact_result <- nested_contact_sim$run(c(0, 1))
stopifnot(
  identical(nested_contact_result$S, c(1, 0)),
  identical(nested_contact_result$I, c(1, 2))
)

# A partial target state can leave the initiator matching the source state. In
# that case the same contact rule must be scheduled again without a duplicate
# pre-scheduling state match.
partial_rate_calls <- 0L
partial_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
partial_sim$addContact(newRandomMixing(
  rate = function(time) {
    partial_rate_calls <<- partial_rate_calls + 1L
    if (partial_rate_calls == 1L) 0.5 else Inf
  },
  type = "partial"
))
partial_sim$addTransition(
  S + I -> list(observed = TRUE) + I ~ "partial"
)
invisible(partial_sim$run(c(0, 1)))
stopifnot(
  identical(partial_rate_calls, 2L),
  identical(getState(partial_sim$agent(1))$observed, TRUE)
)

# The low-level transition boundary rejects malformed callbacks and incomplete
# pairs of contact states with its documented errors.
validation_sim <- ABM:::newSimulation(0)
invalid_callback <- try(
  ABM:::addTransition(
    validation_sim, S, NULL, I, NULL, NULL, 1,
    to_change_callback = 1
  ),
  silent = TRUE
)
missing_contact_to <- try(
  ABM:::addTransition(
    validation_sim, S, I, I, NULL, NULL, NULL
  ),
  silent = TRUE
)
invalid_contact_rate <- try(newRandomMixing(rate = list()), silent = TRUE)
invalid_waiting_time <- try(
  ABM:::addTransition(
    validation_sim, S, NULL, I, NULL, NULL, list()
  ),
  silent = TRUE
)
stopifnot(
  inherits(invalid_callback, "try-error"),
  grepl(
    "to_change_callback must be a function or NULL",
    invalid_callback,
    fixed = TRUE
  ),
  inherits(missing_contact_to, "try-error"),
  grepl("contact to state is NULL", missing_contact_to, fixed = TRUE),
  inherits(invalid_contact_rate, "try-error"),
  grepl(
    "contact rate must be a waiting-time object, function, number, or NULL",
    invalid_contact_rate,
    fixed = TRUE
  ),
  inherits(invalid_waiting_time, "try-error"),
  grepl(
    "waiting_time must be a waiting-time object, function, number, or NULL",
    invalid_waiting_time,
    fixed = TRUE
  )
)

# Defining a rate on both Contact and transition is rejected.
duplicate_rate_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
duplicate_rate_sim$addContact(newRandomMixing(
  type = "physical", rate = function(time) 0
))
duplicate_rate_sim$addTransition(
  S + I -> I + I ~ "physical",
  function(time) 0
)
duplicate_rate_error <- try(duplicate_rate_sim$run(0), silent = TRUE)
stopifnot(
  inherits(duplicate_rate_error, "try-error"),
  grepl("contact rate is defined more than once",
        duplicate_rate_error, fixed = TRUE)
)

# Adding the same contact object more than once is a no-op.
same_contact_sim <- Simulation$new(1)
same_contact <- newRandomMixing(rate = 1, type = "physical")
same_contact_sim$addContact(same_contact)
same_contact_sim$addContact(same_contact)
stopifnot(!inherits(try(same_contact_sim$run(0), silent = TRUE), "try-error"))

# R-defined contacts are likewise retained and attached only once.
build_count <- 0L
DuplicateRContact <- R6::R6Class(
  "DuplicateRContact",
  inherit = Contact,
  public = list(
    contact = function(time, agent) list(),
    addAgent = function(agent) invisible(NULL),
    build = function() {
      build_count <<- build_count + 1L
      invisible(NULL)
    },
    remove = function(agent) invisible(NULL)
  )
)
same_r_contact_sim <- Simulation$new(1)
same_r_contact <- DuplicateRContact$new(rate = 1, type = "r-defined")
same_r_contact_sim$addContact(same_r_contact)
same_r_contact_sim$addContact(same_r_contact)
invisible(same_r_contact_sim$run(0))
stopifnot(identical(build_count, 1L))

# Contacts in a subpopulation are propagated to the simulation registry when
# the subpopulation is attached, and are removed when it leaves.
registry_sim <- Simulation$new(1)
registry_sim$addContact(newRandomMixing(rate = 1, type = "registry"))
registry_subpopulation <- Population$new()
registry_subpopulation$addContact(
  newRandomMixing(rate = 1, type = "registry")
)
registry_sim$addAgent(registry_subpopulation)
registry_duplicate <- try(registry_sim$run(0), silent = TRUE)
stopifnot(
  inherits(registry_duplicate, "try-error"),
  grepl("multiple contact patterns", registry_duplicate, fixed = TRUE)
)
registry_subpopulation$leave()
stopifnot(!inherits(try(registry_sim$run(0), silent = TRUE), "try-error"))

# Contacts added after a subpopulation is attached propagate through the same
# registration hook.
late_registry_sim <- Simulation$new(1)
late_registry_sim$addContact(newRandomMixing(rate = 1, type = "late"))
late_registry_subpopulation <- Population$new()
late_registry_sim$addAgent(late_registry_subpopulation)
late_registry_subpopulation$addContact(
  newRandomMixing(rate = 1, type = "late")
)
late_registry_duplicate <- try(late_registry_sim$run(0), silent = TRUE)
stopifnot(
  inherits(late_registry_duplicate, "try-error"),
  grepl("multiple contact patterns", late_registry_duplicate, fixed = TRUE)
)

# An already-populated contact registry propagates through multiple population
# levels without scanning descendant agents.
deep_registry_sim <- Simulation$new(1)
deep_registry_sim$addContact(newRandomMixing(rate = 1, type = "deep"))
deep_registry_parent <- Population$new()
deep_registry_child <- Population$new()
deep_registry_child$addContact(newRandomMixing(rate = 1, type = "deep"))
deep_registry_parent$addAgent(deep_registry_child)
deep_registry_sim$addAgent(deep_registry_parent)
deep_registry_duplicate <- try(deep_registry_sim$run(0), silent = TRUE)
stopifnot(
  inherits(deep_registry_duplicate, "try-error"),
  grepl("multiple contact patterns", deep_registry_duplicate, fixed = TRUE)
)
deep_registry_parent$leave()
stopifnot(
  !inherits(try(deep_registry_sim$run(0), silent = TRUE), "try-error")
)

# A transition type must identify one contact pattern, not several patterns.
duplicate_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
duplicate_sim$addContact(newRandomMixing(type = "physical"))
duplicate_sim$addContact(newRandomMixing(type = "physical"))
duplicate_sim$addTransition(
  S + I -> I + I ~ "physical",
  function(time) 0
)
duplicate_error <- try(duplicate_sim$run(0), silent = TRUE)
stopifnot(
  inherits(duplicate_error, "try-error"),
  grepl("multiple contact patterns", duplicate_error, fixed = TRUE)
)

# With multiple contact types, omitting ~ is ambiguous and must fail.
ambiguous_sim <- Simulation$new(
  2,
  function(i) list(stage = if (i == 1) "S" else "I")
)
ambiguous_sim$addContact(newRandomMixing(type = "physical"))
ambiguous_sim$addContact(newRandomMixing(type = "social"))
ambiguous_sim$addTransition(
  S + I -> I + I,
  function(time) 0
)
ambiguous_error <- try(ambiguous_sim$run(0), silent = TRUE)
stopifnot(
  inherits(ambiguous_error, "try-error"),
  grepl("exactly one contact pattern", ambiguous_error, fixed = TRUE)
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
