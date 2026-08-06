library(ABM)

population <- Population$new(1)
stopifnot(!"removeAgent" %in% names(population))

# Agent$leave() remains the supported R6 removal API.
agent <- Agent$new(population$agent(1))
agent$leave()
stopifnot(population$size == 0)
