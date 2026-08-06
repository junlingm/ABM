library(ABM)

sim <- Simulation$new(1, function(i) list(value = 1.5))
agent <- sim$agent(1)
sim$addLogger(newStateLogger("value", agent, "value"))

initial <- sim$run(0)
stopifnot(identical(initial$value, 1.5))

setState(agent, list(value = -2.25))
updated <- sim$resume(1)
stopifnot(identical(updated$value, -2.25))
