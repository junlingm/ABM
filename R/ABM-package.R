#' Agent Based Model Simulation Framework
#'
#' This package provides a framework to simulate agent based models that are
#' based on states and events.
#'
#' ## Agent
#' The concept of this framework is agent, which is an object of the [Agent]
#' class. An agent maintains its own state, which is a named R list storing any
#' R values in it. (see [State] ). The main task of an agent is to manage events
#' (see [Event]), and handle them in chronological order.
#'
#' ## Population
#' An object of the [Population] class manages agents and their contacts. The
#' contacts of agents are managed by Contact objects. The main functionality for
#' a contact object is to provide contacts of a given individuals at a given
#' time. For example, [newRandomMixing()] returns such an object that finds a
#' random agent in the population as a contact. the effect of contacts on the
#' states of agents are defined using a state transition rule. Please see
#' ```addTransition``` method of [Simulation] for more details.
#'
#' ## Simulation
#' The [Simulation] class inherits the [Population] class. So a simulation
#' manages agents and their contacts. Thus, the class also inherits the [Agent]
#' class. So a simulation can have its own state, and events attached
#' (scheduled) to it. In addition, it also manages all the transitions, using
#' its ```addTransition``` method. At last, it maintains loggers, which record
#' (or count) the state changes, and report their values at specified times.
#'
#' During a simulation the earliest event in the simulation is picked out,
#' unscheduled (detached), and handled, which potentially causes the state
#' change of the agent (or another agent in the simulation). The state change is
#' then logged by loggers (see [newCounter()] and
#' [newStateLogger()] for more details) that recognize the state
#' change.
#'
#' ## Usage
#' To use this framework, we start by creating a simulation
#' object, populate the simulation with agents (either using the argument in
#' the constructor, or use its ```addAgent``` method), and
#' initialize the agents with their initial states using its ```setState``` method.
#'
#' We then attach ([schedule()]) events to agents (possibly to the populations or
#' the simulation object too), so that these events change the agents' state.
#' For models which agents' states are defined by discrete states, such as the
#' SIR epidemic model, the events are managed by the framework through state
#' transitions, using rules defined by the ```addTransition``` method of
#' the [Simulation] class.
#'
#' At last, we add loggers to the simulation using
#' the [Simulation] class' ```addLogger``` method` and either [newCounter()] or
#' [newStateLogger()]. At last, run the simulation using
#' its ```run``` method, which returns the observations of the loggers
#' at the requested time points as a data.frame object.
#' 
#' For more information and examples, please see the [Wiki](https://github.com/junlingm/ABM/wiki/) pages on Github.
#' 
#' @examples
#' # simulate an SIR model using the Gillespie method
#' # the population size
#' N = 10000
#' # the initial number of infectious agents
#' I0 = 10
#' # the transmission rate
#' beta = 0.4
#' # the recovery rate
#' gamma = 0.2
#' # an waiting time egenerator that handles 0 rate properly
#' wait.exp = function(rate) {
#'   if (rate == 0) Inf else rexp(1, rate)
#' }
#' # this is a function that rescheduled all the events. When the
#' # state changed, the old events are invalid because they are
#' # calculated from the old state. This is possible because the
#' # waiting times are exponentially distributed
#' reschedule = function(time, agent, state) {
#'   clearEvents(agent)
#'   t.inf = time + wait.exp(beta*state$I*state$S/N)
#'   schedule(agent, newEvent(t.inf, handler.infect))
#'   t.rec = time + wait.exp(gamma*state$I)
#'   schedule(agent, newEvent(t.rec, handler.recover))
#' }
#' # The infection event handler
#' # an event handler take 3 arguments
#' #   time is the current simulation time
#' #   sim is an external pointer to the Simulation object.
#' #   agent is the agent that the event is scheduled to
#' handler.infect = function(time, sim, agent) {
#'   x = getState(agent)
#'   x$S = x$S - 1
#'   x$I = x$I + 1
#'   setState(agent, x)
#'   reschedule(time, agent, x)
#' }
#' # The recovery event handler
#' handler.recover = function(time, sim, agent) {
#'   x = getState(agent)
#'   x$R = x$R + 1
#'   x$I = x$I - 1
#'   setState(agent, x)
#'   reschedule(time, agent, x)
#' }
#' # create a new simulation with no agent in it.
#' # note that the simulation object itself is an agent
#' sim = Simulation$new()
#' # the initial state
#' x = list(S=N-I0, I=I0, R=0)
#' sim$state = x
#' # schedule an infection event and a recovery event
#' reschedule(0, sim$get, sim$state)
#' # add state loggers that saves the S, I, and R states
#' sim$addLogger(newStateLogger("S", NULL, "S"))
#' sim$addLogger(newStateLogger("I", NULL, "I"))
#' sim$addLogger(newStateLogger("R", sim$get, "R"))
#' # now the simulation is setup, and is ready to run
#' result = sim$run(0:100)
#' # the result is a data.frame object
#' print(result)
#'
#' # simulate an agent based SEIR model
#' # specify an exponential waiting time for recovery
#' gamma = newExpWaitingTime(0.2)
#' # specify a tansmission rate
#' beta = 0.4
#' # specify a exponentially distributed latent period
#' sigma =newExpWaitingTime(0.5)
#' # the population size
#' N = 10000
#' # create a simulation with N agents, initialize the first 5 with a state "I" 
#' # and the remaining with "S".
#' sim = Simulation$new(N, function(i) if (i <= 5) "I" else "S")
#' # add event loggers that counts the individuals in each state.
#' # the first variable is the name of the counter, the second is
#' # the state for counting. States should be lists. However, for
#' # simplicity, if the state has a single value, then we 
#' # can specify the list as the value, e.g., "S", and the state
#' # is equivalent to list("S")
#' sim$addLogger(newCounter("S", "S"))
#' sim$addLogger(newCounter("E", "E"))
#' sim$addLogger(newCounter("I", "I"))
#' sim$addLogger(newCounter("R", "R"))
#' # create a random mixing contact pattern and attach it to sim
#' m = newRandomMixing()
#' sim$addContact(m)
#' # the transition for leaving latent state anbd becoming infectious
#' sim$addTransition("E"->"I", sigma)
#' # the transition for recovery
#' sim$addTransition("I"->"R", gamma)
#' # the transition for tranmission, which is caused by the contact m
#' # also note that the waiting time can be a number, which is the same
#' # as newExpWaitingTime(beta)
#' sim$addTransition("I" + "S" -> "I" + "E" ~ m, beta)
#' # run the simulation, and get a data.frame object
#' result = sim$run(0:100)
#' print(result)
#'
#' @useDynLib ABM, .registration=TRUE
#' @importFrom R6 R6Class
#' @importFrom Rcpp evalCpp
"_PACKAGE"

#' The state of an agent
#'
#' In this framework, a state is a list, each named component is called a
#' domain. The value of a domain can be any R value. The list can be at most
#' one unnamed value, which corresponds to a domain with no name. This is
#' useful if there is only one domain.
#' 
#' A state can be matched to an R list (called a rule in this case).
#' The state matches the rule if and only if each domain (names of the
#' list) in rule has the same value as in state. The domains in domains of the 
#' state not listed in rule are not matched. In addition, to match to a rule,
#' the domain values must be either a number or a character. This is useful
#' for identifying state changes. See [newCounter()] and 
#' the [Simulation] class' ```addTransition``` method for more details.
#' 
#' @name State
NULL
#' NULL 