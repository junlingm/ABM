# ABM R Package: Agent Based Model Simulation Framework

This package provides a framework to simulate agent based models that are
based on states and events.

# Installation

This R package can be installed using the devtoools::install_github method:
```
install_github("https://github.com/junlingm/ABM.git")
```

# Agent
The concept of this framework is agent, which is an object of the [Agent](https://github.com/junlingm/ABM/wiki/Agent)
class. An agent maintains its own state, which is a named R list storing any
R values in it. (see [State](https://github.com/junlingm/ABM/wiki/State)). The main task of an agent is to manage events
(see [Event](https://github.com/junlingm/ABM/wiki/Event)), and handle them in chronological order.

# Population
An object of the [Population](https://github.com/junlingm/ABM/wiki/Population) class manages agents and their contacts. The
contacts of agents are managed by Contact objects. The main functionality for
a contact object is to provide contacts of a given individuals at a given
time. For example, [newRandomMixing](https://github.com/junlingm/ABM/wiki/newRandomMixing) returns such an object that finds a
random agent in the population as a contact. the effect of contacts on the
states of agents are defined using a state transition rule. Please see
```addTransition``` method of [Simulation](https://github.com/junlingm/ABM/wiki/Simulation) for more details.

# Simulation
The [Simulation](https://github.com/junlingm/ABM/wiki/Simulation) class inherits the [Population](https://github.com/junlingm/ABM/wiki/Population) class. So a simulation
manages agents and their contacts. Thus, the class also inherits the [Agent](https://github.com/junlingm/ABM/wiki/Agent)
class. So a simulation can have its own state, and events attached
(scheduled) to it. In addition, it also manages all the transitions, using
its ```addTransition``` method. At last, it maintains loggers, which record
(or count) the state changes, and report their values at specified times.

During a simulation the earliest event in the simulation is picked out,
unscheduled (detached), and handled, which potentially causes the state
change of the agent (or another agent in the simulation). The state change is
then logged by loggers (see [newCounter](https://github.com/junlingm/ABM/wiki/newCounter) and
[newStateLogger](https://github.com/junlingm/ABM/wiki/newStateLogger) for more details) that recognize the state
change.

# Usage
To use this framework, we start by creating a simulation
object, populate the simulation with agents (either using the argument in
the constructor, or use its ```addAgent``` method), and
initialize the agents with their initial states using its ```setState``` method.

We then attach ([schedule](https://github.com/junlingm/ABM/wiki/schedule)) events to agents (possibly to the populations or
the simulation object too), so that these events change the agents' state.
For models which agents' states are defined by discrete states, such as the
SIR epidemic model, the events are managed by the framework through state
transitions, using rules defined by the ```addTransition``` method of
the [Simulation](https://github.com/junlingm/ABM/wiki/Simulation) class.

At last, we add loggers to the simulation using
the [Simulation](https://github.com/junlingm/ABM/wiki/Simulation) class' ```addLogger``` method` and either [newCounter](https://github.com/junlingm/ABM/wiki/newCounter) or
[newStateLogger](https://github.com/junlingm/ABM/wiki/newStateLogger). At last, run the simulation using
its ```run``` method, which returns the observations of the loggers
at the requested time points as a data.frame object.

# Examples
  1. [Simulate an SIR model using the Gillespie method](https://github.com/junlingm/ABM/wiki/Gillespie-SIR)
  2. [Simulate an agent based SEIR model](https://github.com/junlingm/ABM/wiki/Agent-SIR)
