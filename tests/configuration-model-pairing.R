library(ABM)

contact_slots <- function(degrees, seed = 1) {
  set.seed(seed)
  calls <- 0L
  sim <- Simulation$new(length(degrees), function(i) list(state = "A"))
  network <- newConfigurationModel(function(n) degrees)
  sim$addContact(network)

  A <- list(state = "A")
  waiting_time <- function(time) {
    calls <<- calls + 1L
    Inf
  }
  sim$addTransition(A + A -> A + A ~ network, waiting_time)
  invisible(sim$run(0))
  calls
}

# Two valid stubs form one undirected edge.
stopifnot(contact_slots(c(1L, 1L)) == 2L)

# With three stubs, one pair is connected and the dangling stub is dropped.
stopifnot(contact_slots(c(1L, 1L, 1L)) == 2L)

# A pair of stubs from the same node is dropped as a self-loop.
stopifnot(contact_slots(c(2L, 0L)) == 0L)

# With this seed, both pairs propose the same edge; only one is retained.
stopifnot(contact_slots(c(2L, 2L)) == 2L)
