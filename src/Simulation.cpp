#include "Simulation.h"
#include <cmath>

using namespace Rcpp;

Simulation::Simulation(size_t n)
  : Population(n), _current_time(R_NaN)
{
}

Simulation::~Simulation()
{
  for (auto r : _rules)
    delete r;
}

List Simulation::run(const NumericVector &time)
{
  if (time.size() != 0) {
    _current_time = this->time();
    if (_current_time > time[0]) 
      _current_time = time[0];
    report();
  }
  return resume(time);
}

List Simulation::resume(const NumericVector &time)
{
  size_t n = time.size();
  if (n == 0) return List();
  std::map<std::string, NumericVector> result;
  for (auto c : _loggers)
    result[c->name()] = NumericVector(n);
  size_t i = 0;
  for (auto report : time) {
    while (report > _current_time) {
      this->handle(*this, *this);
      _current_time = _time;
    }
    for (auto c : _loggers)
      result[c->name()][i] = c->report();
    ++i;
  }
  List r;
  r["times"] = time;
  for (auto x : result)
    r[x.first] = x.second;
  return r;
}

void Simulation::stateChanged(Agent &agent, const State &from)
{
  if (!isnan(_current_time)) {
    for (auto c : _loggers)
      c->log(agent, from);
    for (auto r : _rules) {
      if (!from.match(r->from()) && agent.match(r->from())) {
        r->schedule(_current_time, agent);
      }
    }
  }
}

void Simulation::add(std::shared_ptr<Logger> logger)
{
  if (logger) {
    // preventing the same counter added twice
    for (auto l : _loggers) 
      if (l == logger) return;
    _loggers.push_back(logger);
  }
}

void Simulation::add(Transition *rule)
{
  if (rule != NULL) {
    // preventing the same rule added twice
    for (auto r : _rules) 
      if (r == rule) return;
    _rules.push_back(rule);
  }
}

CharacterVector Simulation::classes = CharacterVector::create("Simulation", "Population", "Agent", "Event");

extern "C" {
  SEXP newSimulation(SEXP n)
  {
    int m = (TYPEOF(n) == NILSXP) ? 0 : as<int>(n);
    return XP<Simulation>(std::make_shared<Simulation>(m));
  }

  SEXP runSimulation(SEXP sim, SEXP time)
  {
    return XP<Simulation>(sim)->run(as<NumericVector>(time));
  }

  SEXP resumeSimulation(SEXP sim, SEXP time)
  {
    return XP<Simulation>(sim)->resume(as<NumericVector>(time));
  }

  SEXP addLogger(SEXP sim, SEXP logger)
  {
    XP<Simulation>(sim)->add(XP<Logger>(logger));
    return sim;
  }

  SEXP addTransition(SEXP sim, 
                     SEXP from, SEXP contact_from, 
                     SEXP to, SEXP contact_to, SEXP contact,
                     SEXP waiting_time, 
                     SEXP to_change_callback, SEXP changed_callback)
  {
    XP<Simulation> s(sim);
    List f(from);
    List t(to);
    Nullable<List> cf(contact_from);
    Nullable<List> ct(contact_to);
    Nullable<XP<Contact> > c(contact);
    PWaitingTime w;
    switch (TYPEOF(waiting_time)) {
    case EXTPTRSXP: 
      w = *as<XP<WaitingTime> >(waiting_time).get();
      break;
    case CLOSXP:
    case SPECIALSXP:
    case BUILTINSXP: 
      w = std::make_shared<RWaitingTime>(as<Function>(waiting_time));
      break;
    case REALSXP:
      w = std::make_shared<ExpWaitingTime>(as<double>(waiting_time));
      break;
    default:
      throw std::range_error("waiting_time is invalid");
    }
    Nullable<Function> tc(to_change_callback);
    Nullable<Function> cc(changed_callback);
    if (c.isNull())
      s->add(new Transition(f, t, w, tc, cc));
    else {
      if (cf.isNull())
        std::range_error("contact from state is NULL");
      if (ct.isNull())
        std::range_error("contact to state is NULL");
      s->add(new ContactTransition(f, cf.as(), t, ct.as(), **c.as(), w, tc, cc));
    }
    return sim;
  }
}