# ABM R Package: Agent Based Model Simulation Framework

The ABM package provides a high-performance, flexible framework for agent-based modeling. It has an easy-to-use state transition mechanism, that makes it especially suitable for modeling agent based models. For example, an SEIR model can be implemented in 18 lines. 

In addition, this framework is a general event-based framework.  Yet this framework allows the state of an agent to be quite general, described by a R list taking arbitrary R values. The states are modified by events, which can be easily defined. Thus, it is suitable for a wide range of applications, such as implementing  the Gillespie algorithm.

# Installation

This R package can be installed using the devtoools::install_github method:
```
install_github("https://github.com/junlingm/ABM.git")
```

# Concepts and Usage

See more information on the [Wiki](https://github.com/junlingm/ABM/wiki/)

# Examples
  1. [Simulate an SIR model using the Gillespie method](https://github.com/junlingm/ABM/wiki/Gillespie-SIR)
  2. [Simulate an agent based SEIR model](https://github.com/junlingm/ABM/wiki/Agent-SEIR)
  3. [Simulate a multi-group SEIR model](https://github.com/junlingm/ABM/wiki/Agent-Multigroup)
  4. [Simulate a contact network SIR model](https://github.com/junlingm/ABM/wiki/Agent-Network-SIR)
  5. [Simulate contact tracing on an SIR model](https://github.com/junlingm/ABM/wiki/Contact-Tracing-SIR)

