library(ABM)

# Leaving an unattached agent cannot produce an owning pointer for transfer.
unattached <- Agent$new()
unattached_error <- try(unattached$leave(), silent = TRUE)
stopifnot(
  inherits(unattached_error, "try-error"),
  grepl("agent is not attached to a population", unattached_error, fixed = TRUE)
)

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
  grepl("borrowed handle has expired", expired_agent, fixed = TRUE),
  grepl("borrowed handle has expired", expired_sim, fixed = TRUE)
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
    "borrowed handle has expired",
    expired_transition_agent,
    fixed = TRUE
  )
)

# R-defined contacts are retained by their owning population, while the C++
# bridge keeps only a weak reference to the R6 object.
TestContact <- R6::R6Class(
  "TestContact",
  inherit = Contact,
  public = list(
    contact = function(time, agent) list(),
    addAgent = function(agent) invisible(NULL),
    build = function() invisible(NULL),
    remove = function(agent) invisible(NULL)
  )
)
r_contact_sim <- Simulation$new(1)
r_contact <- TestContact$new(rate = 1)
r_contact_sim$addContact(r_contact)
r_contact <- NULL
invisible(gc())
invisible(r_contact_sim$run(0))

# Handles retained by an R-defined contact follow object and membership
# lifetimes rather than becoming unchecked raw pointers.
saved_added_agent <- NULL
saved_contact_agent <- NULL
saved_removed_agent <- NULL
saved_contact_population <- NULL
LifecycleContact <- R6::R6Class(
  "LifecycleContact",
  inherit = Contact,
  public = list(
    attach = function(population) {
      saved_contact_population <<- population
      self$build()
    },
    contact = function(time, agent) {
      saved_contact_agent <<- agent
      list()
    },
    addAgent = function(agent) {
      saved_added_agent <<- agent
      invisible(NULL)
    },
    build = function() invisible(NULL),
    remove = function(agent) {
      saved_removed_agent <<- agent
      invisible(NULL)
    }
  )
)

S <- list(state = "S")
lifecycle_sim <- Simulation$new(list(S))
lifecycle_contact <- LifecycleContact$new(rate = 1, type = "lifecycle")
lifecycle_sim$addContact(lifecycle_contact)
lifecycle_sim$addTransition(S + S -> S + S ~ "lifecycle")
invisible(lifecycle_sim$run(0))
stopifnot(
  identical(getState(saved_added_agent)$state, "S"),
  identical(getState(saved_contact_agent)$state, "S"),
  identical(getSize(saved_contact_population), 1L)
)

# Removing the agent expires every handle borrowed from that membership, even
# though the owning handle returned by leave() keeps the C++ Agent alive.
removed_agent <- leave(saved_added_agent)
expired_added_agent <- try(getState(saved_added_agent), silent = TRUE)
expired_contact_agent <- try(getState(saved_contact_agent), silent = TRUE)
expired_removed_agent <- try(getState(saved_removed_agent), silent = TRUE)
stopifnot(
  identical(getState(removed_agent)$state, "S"),
  inherits(expired_added_agent, "try-error"),
  inherits(expired_contact_agent, "try-error"),
  inherits(expired_removed_agent, "try-error"),
  grepl("borrowed handle has expired", expired_added_agent, fixed = TRUE),
  grepl("borrowed handle has expired", expired_contact_agent, fixed = TRUE),
  grepl("borrowed handle has expired", expired_removed_agent, fixed = TRUE)
)

# The population handle retained by attach() remains valid until the
# Population is destroyed, independently of the R contact's own lifetime.
lifecycle_sim <- NULL
invisible(gc())
expired_contact_population <- try(
  getSize(saved_contact_population),
  silent = TRUE
)
stopifnot(
  inherits(expired_contact_population, "try-error"),
  grepl(
    "borrowed handle has expired",
    expired_contact_population,
    fixed = TRUE
  )
)

# Population destruction also invalidates membership handles when another
# owning external pointer keeps the Agent object itself alive.
destroyed_membership_agent <- NULL
DestructorContact <- R6::R6Class(
  "DestructorContact",
  inherit = Contact,
  public = list(
    contact = function(time, agent) list(),
    addAgent = function(agent) {
      destroyed_membership_agent <<- agent
      invisible(NULL)
    },
    build = function() invisible(NULL),
    remove = function(agent) invisible(NULL)
  )
)
destruction_sim <- Simulation$new(list(S))
destruction_contact <- DestructorContact$new(rate = 1)
destruction_sim$addContact(destruction_contact)
surviving_agent <- destruction_sim$agent(1)
destruction_sim <- NULL
invisible(gc())
expired_destroyed_membership <- try(
  getState(destroyed_membership_agent),
  silent = TRUE
)
detached_surviving_agent <- try(leave(surviving_agent), silent = TRUE)
stopifnot(
  identical(getState(surviving_agent)$state, "S"),
  inherits(expired_destroyed_membership, "try-error"),
  grepl(
    "borrowed handle has expired",
    expired_destroyed_membership,
    fixed = TRUE
  ),
  inherits(detached_surviving_agent, "try-error"),
  grepl(
    "agent is not attached to a population",
    detached_surviving_agent,
    fixed = TRUE
  )
)

# A surviving Agent can be added to another Population after its original
# Population is destroyed. Its calendar must not retain the destroyed owner.
replacement_sim <- Simulation$new()
replacement_sim$addAgent(surviving_agent)
stopifnot(
  replacement_sim$size == 1,
  identical(getState(surviving_agent)$state, "S")
)
