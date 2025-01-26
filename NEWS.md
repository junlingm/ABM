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
  
  
