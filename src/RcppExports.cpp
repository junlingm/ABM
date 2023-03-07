// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include "../inst/include/ABM.h"
#include <Rcpp.h>

using namespace Rcpp;

#ifdef RCPP_USE_GLOBAL_ROSTREAM
Rcpp::Rostream<true>&  Rcpp::Rcout = Rcpp::Rcpp_cout_get();
Rcpp::Rostream<false>& Rcpp::Rcerr = Rcpp::Rcpp_cerr_get();
#endif

// newAgent
XP<Agent> newAgent(SEXP state);
RcppExport SEXP _ABM_newAgent(SEXP stateSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< SEXP >::type state(stateSEXP);
    rcpp_result_gen = Rcpp::wrap(newAgent(state));
    return rcpp_result_gen;
END_RCPP
}
// getID
int getID(XP<Agent> agent);
RcppExport SEXP _ABM_getID(SEXP agentSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    rcpp_result_gen = Rcpp::wrap(getID(agent));
    return rcpp_result_gen;
END_RCPP
}
// getState
List getState(XP<Agent> agent);
RcppExport SEXP _ABM_getState(SEXP agentSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    rcpp_result_gen = Rcpp::wrap(getState(agent));
    return rcpp_result_gen;
END_RCPP
}
// schedule
XP<Agent> schedule(XP<Agent> agent, XP<Event> event);
RcppExport SEXP _ABM_schedule(SEXP agentSEXP, SEXP eventSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    Rcpp::traits::input_parameter< XP<Event> >::type event(eventSEXP);
    rcpp_result_gen = Rcpp::wrap(schedule(agent, event));
    return rcpp_result_gen;
END_RCPP
}
// unschedule
XP<Agent> unschedule(XP<Agent> agent, XP<Event> event);
RcppExport SEXP _ABM_unschedule(SEXP agentSEXP, SEXP eventSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    Rcpp::traits::input_parameter< XP<Event> >::type event(eventSEXP);
    rcpp_result_gen = Rcpp::wrap(unschedule(agent, event));
    return rcpp_result_gen;
END_RCPP
}
// clearEvents
XP<Agent> clearEvents(XP<Agent> agent);
RcppExport SEXP _ABM_clearEvents(SEXP agentSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    rcpp_result_gen = Rcpp::wrap(clearEvents(agent));
    return rcpp_result_gen;
END_RCPP
}
// setState
XP<Agent> setState(XP<Agent> agent, SEXP value);
RcppExport SEXP _ABM_setState(SEXP agentSEXP, SEXP valueSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    Rcpp::traits::input_parameter< SEXP >::type value(valueSEXP);
    rcpp_result_gen = Rcpp::wrap(setState(agent, value));
    return rcpp_result_gen;
END_RCPP
}
// newRandomMixing
XP<Contact> newRandomMixing();
RcppExport SEXP _ABM_newRandomMixing() {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    rcpp_result_gen = Rcpp::wrap(newRandomMixing());
    return rcpp_result_gen;
END_RCPP
}
// newContact
XP<Contact> newContact(Environment r6);
RcppExport SEXP _ABM_newContact(SEXP r6SEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Environment >::type r6(r6SEXP);
    rcpp_result_gen = Rcpp::wrap(newContact(r6));
    return rcpp_result_gen;
END_RCPP
}
// newCounter
XP<Counter> newCounter(std::string name, List from, Nullable<List> to, int initial);
RcppExport SEXP _ABM_newCounter(SEXP nameSEXP, SEXP fromSEXP, SEXP toSEXP, SEXP initialSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type name(nameSEXP);
    Rcpp::traits::input_parameter< List >::type from(fromSEXP);
    Rcpp::traits::input_parameter< Nullable<List> >::type to(toSEXP);
    Rcpp::traits::input_parameter< int >::type initial(initialSEXP);
    rcpp_result_gen = Rcpp::wrap(newCounter(name, from, to, initial));
    return rcpp_result_gen;
END_RCPP
}
// newStateLogger
XP<StateLogger> newStateLogger(std::string name, Nullable<XP<Agent> > agent, std::string state);
RcppExport SEXP _ABM_newStateLogger(SEXP nameSEXP, SEXP agentSEXP, SEXP stateSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< std::string >::type name(nameSEXP);
    Rcpp::traits::input_parameter< Nullable<XP<Agent> > >::type agent(agentSEXP);
    Rcpp::traits::input_parameter< std::string >::type state(stateSEXP);
    rcpp_result_gen = Rcpp::wrap(newStateLogger(name, agent, state));
    return rcpp_result_gen;
END_RCPP
}
// newEvent
XP<Event> newEvent(double time, Function handler);
RcppExport SEXP _ABM_newEvent(SEXP timeSEXP, SEXP handlerSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< double >::type time(timeSEXP);
    Rcpp::traits::input_parameter< Function >::type handler(handlerSEXP);
    rcpp_result_gen = Rcpp::wrap(newEvent(time, handler));
    return rcpp_result_gen;
END_RCPP
}
// getTime
double getTime(XP<Event> event);
RcppExport SEXP _ABM_getTime(SEXP eventSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Event> >::type event(eventSEXP);
    rcpp_result_gen = Rcpp::wrap(getTime(event));
    return rcpp_result_gen;
END_RCPP
}
// newConfigurationModel
XP<ConfigurationModel> newConfigurationModel(Function rng);
RcppExport SEXP _ABM_newConfigurationModel(SEXP rngSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Function >::type rng(rngSEXP);
    rcpp_result_gen = Rcpp::wrap(newConfigurationModel(rng));
    return rcpp_result_gen;
END_RCPP
}
// newPopulation
XP<Population> newPopulation(int n);
RcppExport SEXP _ABM_newPopulation(SEXP nSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< int >::type n(nSEXP);
    rcpp_result_gen = Rcpp::wrap(newPopulation(n));
    return rcpp_result_gen;
END_RCPP
}
// addAgent
XP<Population> addAgent(XP<Population> population, XP<Agent> agent);
RcppExport SEXP _ABM_addAgent(SEXP populationSEXP, SEXP agentSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Population> >::type population(populationSEXP);
    Rcpp::traits::input_parameter< XP<Agent> >::type agent(agentSEXP);
    rcpp_result_gen = Rcpp::wrap(addAgent(population, agent));
    return rcpp_result_gen;
END_RCPP
}
// getSize
int getSize(XP<Population> population);
RcppExport SEXP _ABM_getSize(SEXP populationSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Population> >::type population(populationSEXP);
    rcpp_result_gen = Rcpp::wrap(getSize(population));
    return rcpp_result_gen;
END_RCPP
}
// getAgent
XP<Agent> getAgent(XP<Population> population, int i);
RcppExport SEXP _ABM_getAgent(SEXP populationSEXP, SEXP iSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Population> >::type population(populationSEXP);
    Rcpp::traits::input_parameter< int >::type i(iSEXP);
    rcpp_result_gen = Rcpp::wrap(getAgent(population, i));
    return rcpp_result_gen;
END_RCPP
}
// addContact
XP<Population> addContact(XP<Population> population, XP<Contact> contact);
RcppExport SEXP _ABM_addContact(SEXP populationSEXP, SEXP contactSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Population> >::type population(populationSEXP);
    Rcpp::traits::input_parameter< XP<Contact> >::type contact(contactSEXP);
    rcpp_result_gen = Rcpp::wrap(addContact(population, contact));
    return rcpp_result_gen;
END_RCPP
}
// setStates
XP<Population> setStates(XP<Population> population, SEXP states);
RcppExport SEXP _ABM_setStates(SEXP populationSEXP, SEXP statesSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Population> >::type population(populationSEXP);
    Rcpp::traits::input_parameter< SEXP >::type states(statesSEXP);
    rcpp_result_gen = Rcpp::wrap(setStates(population, states));
    return rcpp_result_gen;
END_RCPP
}
// newSimulation
XP<Simulation> newSimulation(int n);
RcppExport SEXP _ABM_newSimulation(SEXP nSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< int >::type n(nSEXP);
    rcpp_result_gen = Rcpp::wrap(newSimulation(n));
    return rcpp_result_gen;
END_RCPP
}
// runSimulation
List runSimulation(XP<Simulation> sim, NumericVector time);
RcppExport SEXP _ABM_runSimulation(SEXP simSEXP, SEXP timeSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Simulation> >::type sim(simSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type time(timeSEXP);
    rcpp_result_gen = Rcpp::wrap(runSimulation(sim, time));
    return rcpp_result_gen;
END_RCPP
}
// resumeSimulation
List resumeSimulation(XP<Simulation> sim, NumericVector time);
RcppExport SEXP _ABM_resumeSimulation(SEXP simSEXP, SEXP timeSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Simulation> >::type sim(simSEXP);
    Rcpp::traits::input_parameter< NumericVector >::type time(timeSEXP);
    rcpp_result_gen = Rcpp::wrap(resumeSimulation(sim, time));
    return rcpp_result_gen;
END_RCPP
}
// addLogger
XP<Simulation> addLogger(XP<Simulation> sim, XP<Logger> logger);
RcppExport SEXP _ABM_addLogger(SEXP simSEXP, SEXP loggerSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Simulation> >::type sim(simSEXP);
    Rcpp::traits::input_parameter< XP<Logger> >::type logger(loggerSEXP);
    rcpp_result_gen = Rcpp::wrap(addLogger(sim, logger));
    return rcpp_result_gen;
END_RCPP
}
// addTransition
XP<Simulation> addTransition(XP<Simulation> sim, List from, Nullable<List> contact_from, List to, Nullable<List> contact_to, Nullable<XP<Contact> > contact, SEXP waiting_time, Nullable<Function> to_change_callback, Nullable<Function> changed_callback);
RcppExport SEXP _ABM_addTransition(SEXP simSEXP, SEXP fromSEXP, SEXP contact_fromSEXP, SEXP toSEXP, SEXP contact_toSEXP, SEXP contactSEXP, SEXP waiting_timeSEXP, SEXP to_change_callbackSEXP, SEXP changed_callbackSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<Simulation> >::type sim(simSEXP);
    Rcpp::traits::input_parameter< List >::type from(fromSEXP);
    Rcpp::traits::input_parameter< Nullable<List> >::type contact_from(contact_fromSEXP);
    Rcpp::traits::input_parameter< List >::type to(toSEXP);
    Rcpp::traits::input_parameter< Nullable<List> >::type contact_to(contact_toSEXP);
    Rcpp::traits::input_parameter< Nullable<XP<Contact> > >::type contact(contactSEXP);
    Rcpp::traits::input_parameter< SEXP >::type waiting_time(waiting_timeSEXP);
    Rcpp::traits::input_parameter< Nullable<Function> >::type to_change_callback(to_change_callbackSEXP);
    Rcpp::traits::input_parameter< Nullable<Function> >::type changed_callback(changed_callbackSEXP);
    rcpp_result_gen = Rcpp::wrap(addTransition(sim, from, contact_from, to, contact_to, contact, waiting_time, to_change_callback, changed_callback));
    return rcpp_result_gen;
END_RCPP
}
// stateMatch
bool stateMatch(List state, SEXP rule);
RcppExport SEXP _ABM_stateMatch(SEXP stateSEXP, SEXP ruleSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< List >::type state(stateSEXP);
    Rcpp::traits::input_parameter< SEXP >::type rule(ruleSEXP);
    rcpp_result_gen = Rcpp::wrap(stateMatch(state, rule));
    return rcpp_result_gen;
END_RCPP
}
// newExpWaitingTime
XP<WaitingTime> newExpWaitingTime(double rate);
RcppExport SEXP _ABM_newExpWaitingTime(SEXP rateSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< double >::type rate(rateSEXP);
    rcpp_result_gen = Rcpp::wrap(newExpWaitingTime(rate));
    return rcpp_result_gen;
END_RCPP
}
// newGammaWaitingTime
XP<WaitingTime> newGammaWaitingTime(double shape, double scale);
RcppExport SEXP _ABM_newGammaWaitingTime(SEXP shapeSEXP, SEXP scaleSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< double >::type shape(shapeSEXP);
    Rcpp::traits::input_parameter< double >::type scale(scaleSEXP);
    rcpp_result_gen = Rcpp::wrap(newGammaWaitingTime(shape, scale));
    return rcpp_result_gen;
END_RCPP
}
// newRWaitingTime
XP<WaitingTime> newRWaitingTime(Function rng);
RcppExport SEXP _ABM_newRWaitingTime(SEXP rngSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< Function >::type rng(rngSEXP);
    rcpp_result_gen = Rcpp::wrap(newRWaitingTime(rng));
    return rcpp_result_gen;
END_RCPP
}
// getWaitingTime
double getWaitingTime(XP<WaitingTime> generator, double time);
RcppExport SEXP _ABM_getWaitingTime(SEXP generatorSEXP, SEXP timeSEXP) {
BEGIN_RCPP
    Rcpp::RObject rcpp_result_gen;
    Rcpp::RNGScope rcpp_rngScope_gen;
    Rcpp::traits::input_parameter< XP<WaitingTime> >::type generator(generatorSEXP);
    Rcpp::traits::input_parameter< double >::type time(timeSEXP);
    rcpp_result_gen = Rcpp::wrap(getWaitingTime(generator, time));
    return rcpp_result_gen;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_ABM_newAgent", (DL_FUNC) &_ABM_newAgent, 1},
    {"_ABM_getID", (DL_FUNC) &_ABM_getID, 1},
    {"_ABM_getState", (DL_FUNC) &_ABM_getState, 1},
    {"_ABM_schedule", (DL_FUNC) &_ABM_schedule, 2},
    {"_ABM_unschedule", (DL_FUNC) &_ABM_unschedule, 2},
    {"_ABM_clearEvents", (DL_FUNC) &_ABM_clearEvents, 1},
    {"_ABM_setState", (DL_FUNC) &_ABM_setState, 2},
    {"_ABM_newRandomMixing", (DL_FUNC) &_ABM_newRandomMixing, 0},
    {"_ABM_newContact", (DL_FUNC) &_ABM_newContact, 1},
    {"_ABM_newCounter", (DL_FUNC) &_ABM_newCounter, 4},
    {"_ABM_newStateLogger", (DL_FUNC) &_ABM_newStateLogger, 3},
    {"_ABM_newEvent", (DL_FUNC) &_ABM_newEvent, 2},
    {"_ABM_getTime", (DL_FUNC) &_ABM_getTime, 1},
    {"_ABM_newConfigurationModel", (DL_FUNC) &_ABM_newConfigurationModel, 1},
    {"_ABM_newPopulation", (DL_FUNC) &_ABM_newPopulation, 1},
    {"_ABM_addAgent", (DL_FUNC) &_ABM_addAgent, 2},
    {"_ABM_getSize", (DL_FUNC) &_ABM_getSize, 1},
    {"_ABM_getAgent", (DL_FUNC) &_ABM_getAgent, 2},
    {"_ABM_addContact", (DL_FUNC) &_ABM_addContact, 2},
    {"_ABM_setStates", (DL_FUNC) &_ABM_setStates, 2},
    {"_ABM_newSimulation", (DL_FUNC) &_ABM_newSimulation, 1},
    {"_ABM_runSimulation", (DL_FUNC) &_ABM_runSimulation, 2},
    {"_ABM_resumeSimulation", (DL_FUNC) &_ABM_resumeSimulation, 2},
    {"_ABM_addLogger", (DL_FUNC) &_ABM_addLogger, 2},
    {"_ABM_addTransition", (DL_FUNC) &_ABM_addTransition, 9},
    {"_ABM_stateMatch", (DL_FUNC) &_ABM_stateMatch, 2},
    {"_ABM_newExpWaitingTime", (DL_FUNC) &_ABM_newExpWaitingTime, 1},
    {"_ABM_newGammaWaitingTime", (DL_FUNC) &_ABM_newGammaWaitingTime, 2},
    {"_ABM_newRWaitingTime", (DL_FUNC) &_ABM_newRWaitingTime, 1},
    {"_ABM_getWaitingTime", (DL_FUNC) &_ABM_getWaitingTime, 2},
    {NULL, NULL, 0}
};

RcppExport void R_init_ABM(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
