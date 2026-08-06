library(ABM)

initializer <- function(i) list(index = i)
indexes <- function(population) {
  vapply(
    seq_len(population$size),
    function(i) getState(population$agent(i))$index,
    numeric(1)
  )
}

population <- Population$new(3, initializer)
stopifnot(identical(indexes(population), c(1, 2, 3)))

simulation <- Simulation$new(3, initializer)
stopifnot(identical(indexes(simulation), c(1, 2, 3)))
