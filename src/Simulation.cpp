#include "../inst/include/Simulation.h"
#include <cmath>

using namespace Rcpp;

Simulation::Simulation(size_t n, Rcpp::Nullable<Rcpp::Function> initializer)
  : Population(n, initializer), _current_time(R_NaN)
{
  for (auto a : _agents)
    a->attached(*this);
}

Simulation::Simulation(List states)
  : Population(states), _current_time(R_NaN)
{
  for (auto a : _agents)
    a->attached(*this);
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
    while (report > _time) {
      _current_time = _time;
      this->handle(*this, *this);
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
  if (!std::isnan(_current_time)) {
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

Simulation *Simulation::simulation()
{
  return this;
}

const Simulation *Simulation::simulation() const
{
  return this;
}

CharacterVector Simulation::classes = CharacterVector::create("Simulation", "Population", "Agent", "Event");

// [[Rcpp::export]]
XP<Simulation> newSimulation(SEXP n, Nullable<Function> initializer = R_NilValue)
{
  if (n == R_NilValue)
    return XP<Simulation>(std::make_shared<Simulation>());
  if (Rf_isNumeric(n)) {
    int N = as<int>(n); 
    if (N < 0) N = 0;
    return XP<Simulation>(std::make_shared<Simulation>(N, initializer));
  }
  if (Rf_isVector(n))
    return XP<Simulation>(std::make_shared<Simulation>(List(n)));
  stop("n must be an integer or a list");
}

// [[Rcpp::export]]
List runSimulation(XP<Simulation> sim, NumericVector time)
{
  return sim->run(time);
}

// [[Rcpp::export]]
List resumeSimulation(XP<Simulation> sim, NumericVector time)
{
  return sim->resume(time);
}

// [[Rcpp::export]]
void addLogger(XP<Simulation> sim, XP<Logger> logger)
{
  sim->add(logger);
}

// [[Rcpp::export]]
void addTransition(
    XP<Simulation> sim, 
    List from, Nullable<List> contact_from, 
    List to, Nullable<List> contact_to, Nullable<XP<Contact> > contact,
    SEXP waiting_time, 
    Nullable<Function> to_change_callback = R_NilValue, 
    Nullable<Function> changed_callback = R_NilValue)
{
  PWaitingTime w;
  if (TYPEOF(waiting_time) == EXTPTRSXP)
    w = as<XP<WaitingTime> >(waiting_time);
  else if (Rf_isFunction(waiting_time)) 
    w = std::make_shared<RWaitingTime>(as<Function>(waiting_time));
  else if (Rf_isNumeric(waiting_time))
    w = std::make_shared<ExpWaitingTime>(as<double>(waiting_time));
  else
    throw std::range_error("waiting_time is invalid");
  if (to_change_callback != R_NilValue && !Rf_isFunction(to_change_callback))
    std::range_error("to_change_callback must be a function or NULL");
  if (changed_callback != R_NilValue && !Rf_isFunction(changed_callback))
    std::range_error("changed_callback must be a function or NULL");
  if (contact.isNull())
    sim->add(new Transition(from, to, w, to_change_callback, changed_callback));
  else {
    if (contact_from.isNull())
      std::range_error("contact from state is NULL");
    if (contact_to.isNull())
      std::range_error("contact to state is NULL");
    sim->add(new ContactTransition(
        from, contact_from.as(), to, contact_to.as(),
        **contact.as(), w, to_change_callback, changed_callback));
  }
}
