library(ABM)

# The public argument is a scale, while the cached C++ generator stores a rate.
# Compare across the 10,000-value cache boundary.
n <- 10001L
shape <- 2.5
scale <- 3

set.seed(42)
expected <- rgamma(n, shape = shape, scale = scale)

set.seed(42)
generator <- newGammaWaitingTime(shape, scale)
observed <- vapply(
  seq_len(n),
  function(i) getWaitingTime(generator, 0),
  numeric(1)
)

stopifnot(identical(observed, expected))

# Preserve the limiting behavior at zero and infinite scale.
stopifnot(
  getWaitingTime(newGammaWaitingTime(shape, 0), 0) == 0,
  is.infinite(getWaitingTime(newGammaWaitingTime(shape, Inf), 0))
)
