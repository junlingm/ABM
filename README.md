# ABM R Package: Agent Based Model Simulation Framework

The ABM package provides a high-performance, flexible framework for agent-based modeling. It has an easy-to-use state transition mechanism, that makes it especially suitable for modeling agent based models. For example, an SEIR model can be implemented in 18 lines. 

In addition, this framework is a general event-based framework.  Yet this framework allows the state of an agent to be quite general, described by a R list taking arbitrary R values. The states are modified by events, which can be easily defined. Thus, it is suitable for a wide range of applications, such as implementing  the Gillespie algorithm.

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

See more information on the [Wiki page](https://github.com/junlingm/ABM/wiki/#Usage)

# Examples
  1. [Simulate an SIR model using the Gillespie method](https://github.com/junlingm/ABM/wiki/Gillespie-SIR)
  2. [Simulate an agent based SEIR model](https://github.com/junlingm/ABM/wiki/Agent-SIR)
  3. [Simulate a multi-group SEIR model](https://github.com/junlingm/ABM/wiki/Agent-Multigroup)
  4. [Simulate a contact network SIR model](https://github.com/junlingm/ABM/wiki/Agent-Network-SIR)

