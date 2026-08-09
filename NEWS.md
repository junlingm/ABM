# Version 0.5.1
* External pointers now validate their capability tags and callback-scoped
  handles expire safely after the callback returns.
* Agent removal now rejects unattached agents and validates population
  ownership, preserving agent state when removal fails.
* Fixed leaks from transition callbacks and removed the cross-language cycle
  between R-defined contacts and their C++ bridges.
* Added regression tests for external-pointer and contact lifecycle safety.

# Version 0.5.0
* Added event-driven transition logging with `inc()` and `dec()` operations,
  optional filters, and `NULL` as the default transition logging setting.
* `Simulation$addLogger()` now accepts a state name with an optional output
  name. Explicit `StateLogger` registration and `newCounter()` are deprecated.
* Agent state updates merge named domains, and legacy state-change logging now
  uses pre- and post-change notifications without cloning the full state.
* Simulation state variables are updated in place, avoiding unnecessary state
  notifications for event-driven counter updates.

# Version 0.4.4
* Fixed configuration-model networks when agents are added or removed after
  network construction. New edges now use the sampled degree-weighted node,
  and removals consistently use population-local indexes.
* Configuration-model stubs are now paired without replacement. Self-loops,
  duplicate edges, and the final dangling stub from an odd total are dropped.
* Moving an agent now removes it from its former population and transfers its
  calendar membership correctly.
* Simulation and population constructor initializers now receive one-based
  indexes, and simulation agent ID counters are initialized deterministically.
* Resuming through an interval without events now advances the simulation
  clock to the requested report time.
* Gamma waiting times now interpret their second parameter as a scale.
* State loggers now return doubles, and counters support numeric states.
* Removed the stale `Population$removeAgent()` R6 method. Agents are removed
  from their current population with `Agent$leave()` or `leave()`.

# Version 0.4.3
* Fixed a bug that prevented the parsingstate transitions such as list(x="a", y="b")->list("x="A", y="b").
* Fixed a possible memory leak suggested by valgrind.
* Use cached gamma random number generators to reimplement newGammaWaitingTime.

# Version 0.4.2
* Fixed a bug that caused the simulation to be incorrect after exhausting random 
numbers in a cache. This is because refilling the cached random numbers did not
correctly setting the RNGScope as required by Rcpp.
* Fixed a bug that logged the states at wrong simulation time.
* Do not schedule a transition event if the event will never happen (event time is 
infinity)

# Version 0.4.1
* Fixed a memory leak bug. This resolved the valgrind check error
* Changed the internal handling of agent ID, so that agent ID is unique across all populations.
* Fixed a bug that caused agent initializers in the constructor of Simulation to fail.
* Fixed a bug that prevent the simulation results to be reproducible.

# Version 0.4
* This release have some API changes regarding the Rcpp manipulation functions, such as schedule, setState and addLogger etc. These functions used to return the object itself for action chaining. Now they do not return a value. However, action changing is still possible using the R6 objects.
* Fixed the missing ABM-package documentation.
* Fixed a minor memory leak problem caused by the StateLogger class.

# Version 0.3

Remove agent from a population. This allows agent deaths and moving between populations.

# Version 0.2
  
initializers for the state of agents in the population

# Version 0.1
  
Initial implementation
  
  
