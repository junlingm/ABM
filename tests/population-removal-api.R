library(ABM)

population <- Population$new(1)
stopifnot(!"removeAgent" %in% names(population))

# Agent$leave() remains the supported R6 removal API.
agent <- Agent$new(population$agent(1))
agent$leave()
stopifnot(population$size == 0)

# Public agent lookup validates its one-based index instead of indexing the
# native vector out of bounds.
indexed_population <- Population$new(1)
zero_index <- try(indexed_population$agent(0), silent = TRUE)
large_index <- try(indexed_population$agent(2), silent = TRUE)
stopifnot(
  inherits(zero_index, "try-error"),
  inherits(large_index, "try-error"),
  grepl("agent index is out of range", zero_index, fixed = TRUE),
  grepl("agent index is out of range", large_index, fixed = TRUE)
)

# Population containment must remain acyclic.
self_population <- Population$new()
self_add <- try(self_population$addAgent(self_population), silent = TRUE)
parent_population <- Population$new()
child_population <- Population$new()
parent_population$addAgent(child_population)
ancestor_add <- try(
  child_population$addAgent(parent_population),
  silent = TRUE
)
stopifnot(
  inherits(self_add, "try-error"),
  inherits(ancestor_add, "try-error"),
  grepl("cannot add a population", self_add, fixed = TRUE),
  grepl("cannot add a population", ancestor_add, fixed = TRUE),
  identical(self_population$size, 0L),
  identical(parent_population$size, 1L),
  identical(child_population$size, 0L)
)
