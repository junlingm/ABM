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
