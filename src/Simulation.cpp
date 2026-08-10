#include "../inst/include/Simulation.h"
#include <cmath>

using namespace Rcpp;

Simulation::Simulation(size_t n, Rcpp::Nullable<Rcpp::Function> initializer)
  : Population(n, initializer), _current_time(R_NaN), _next_id(0)
{
  for (auto a : _agents)
    a->attached(*this);
}

Simulation::Simulation(List states)
  : Population(states), _current_time(R_NaN), _next_id(0)
{
  for (auto a : _agents)
    a->attached(*this);
}

Simulation::~Simulation()
{
  for (auto r : _rules)
    delete r;
}

void Simulation::report()
{
  prepareContacts();
  resetContactRates(*this);
  std::set<std::string> types;
  collectContactTypes(*this, types);
  for (auto rule : _rules) {
    ContactTransition *transition = dynamic_cast<ContactTransition*>(rule);
    if (transition && !transition->contactType() && types.size() != 1)
      stop("a contact transition without a type requires exactly one "
           "contact pattern");
    if (transition)
      transition->clearContact();
  }
  registerTransitions(*this, types);
  Population::report();
}

void Simulation::resetContactRates(Population &population)
{
  for (auto &contact : population._contacts)
    contact->resetLegacyRate();
  for (auto &agent : population._agents) {
    Population *nested = dynamic_cast<Population*>(agent.get());
    if (nested)
      resetContactRates(*nested);
  }
}

void Simulation::collectContactTypes(
    Population &population, std::set<std::string> &types)
{
  for (auto &contact : population._contacts) {
    if (!types.insert(contact->type()).second)
      stop("multiple contact patterns have the same type");
  }
  for (auto &agent : population._agents) {
    Population *nested = dynamic_cast<Population*>(agent.get());
    if (nested)
      collectContactTypes(*nested, types);
  }
}

void Simulation::registerTransitions(
    Population &population, const std::set<std::string> &types)
{
  for (auto &contact : population._contacts) {
    for (auto rule : _rules) {
      ContactTransition *transition = dynamic_cast<ContactTransition*>(rule);
      if (transition &&
          ((!transition->contactType() && types.size() == 1) ||
           (transition->contactType() &&
            contact->type() == *transition->contactType())))
      {
        if (transition->waitingTime())
          contact->assignLegacyRate(transition->waitingTime());
        transition->addContact(*contact);
      }
    }
    if (!contact->hasRate())
      warning("Contact has no rate; specify rate when creating the Contact; "
              "transition-level rates are deprecated");
  }
  for (auto &agent : population._agents) {
    Population *nested = dynamic_cast<Population*>(agent.get());
    if (nested)
      registerTransitions(*nested, types);
  }
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
    _current_time = report;
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
      ContactTransition *contact = dynamic_cast<ContactTransition*>(r);
      if (!from.match(r->from()) && agent.match(r->from())) {
        if (contact)
          contact->schedule(_current_time, agent);
        else
          r->schedule(_current_time, agent);
      }
    }
  }
}

void Simulation::stateChanging(Agent &agent, const Rcpp::List &state)
{
  _pending_loggers.clear();
  _pending_rules.clear();
  if (!std::isnan(_current_time)) {
    for (auto logger : _loggers)
      if (logger->stateChanging(agent, state))
        _pending_loggers.push_back(logger.get());
    for (auto rule : _rules)
      if (!agent.match(rule->from()))
        _pending_rules.push_back(rule);
  }
}

void Simulation::stateChanged(Agent &agent)
{
  if (!std::isnan(_current_time)) {
    for (auto logger : _pending_loggers)
      logger->stateChanged(agent);
    for (auto rule : _pending_rules) {
      if (!agent.match(rule->from()))
        continue;
      ContactTransition *contact = dynamic_cast<ContactTransition*>(rule);
      if (contact) {
        contact->schedule(_current_time, agent);
      } else rule->schedule(_current_time, agent);
    }
  }
  _pending_loggers.clear();
  _pending_rules.clear();
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

void Simulation::change(const std::string &name, double delta)
{
  List current = state();
  R_xlen_t position = current.findName(name);
  if (position < 0)
    stop("simulation state variable not found: ", name);
  SEXP old = current[position];
  if ((TYPEOF(old) != INTSXP && TYPEOF(old) != REALSXP) ||
      Rf_length(old) != 1)
    stop("simulation state variable must be a numeric scalar: ", name);
  double value = as<double>(current[position]);
  List update(1);
  update[0] = value + delta;
  update.attr("names") = CharacterVector::create(name);
  set(update, false);
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

static std::string contactType(SEXP contact)
{
  if (TYPEOF(contact) == STRSXP) {
    if (Rf_xlength(contact) != 1 || STRING_ELT(contact, 0) == NA_STRING)
      stop("contact type must be a single, non-missing string");
    std::string type = as<std::string>(contact);
    if (type.empty())
      stop("contact type must not be empty");
    return type;
  }
  if (TYPEOF(contact) == EXTPTRSXP) {
    warning("Passing a Contact object to addTransition() is deprecated; "
            "use its contact type string instead");
    XP<Contact> pointer(contact);
    return pointer->type();
  }
  stop("contact must be a type string or a Contact object");
}

// [[Rcpp::export]]
void addTransition(
    XP<Simulation> sim, 
    List from, Nullable<List> contact_from, 
    List to, Nullable<List> contact_to, SEXP contact,
    SEXP waiting_time, 
    Nullable<Function> to_change_callback = R_NilValue, 
    Nullable<Function> changed_callback = R_NilValue,
    Nullable<List> logging = R_NilValue)
{
  bool contact_rule = !contact_from.isNull() || !contact_to.isNull();
  PWaitingTime w;
  if (waiting_time == R_NilValue) {
    if (!contact_rule)
      stop("waiting_time is required for a spontaneous transition");
  } else if (TYPEOF(waiting_time) == EXTPTRSXP)
    w = as<XP<WaitingTime> >(waiting_time);
  else if (Rf_isFunction(waiting_time)) 
    w = std::make_shared<RWaitingTime>(as<Function>(waiting_time));
  else if (Rf_isNumeric(waiting_time))
    w = std::make_shared<ExpWaitingTime>(as<double>(waiting_time));
  else
    throw std::range_error("waiting_time is invalid");
  if (contact_rule && waiting_time != R_NilValue)
    warning("Supplying waiting.time for a contact transition is deprecated; "
            "specify the rate on the Contact instead");
  if (to_change_callback != R_NilValue && !Rf_isFunction(to_change_callback))
    std::range_error("to_change_callback must be a function or NULL");
  if (changed_callback != R_NilValue && !Rf_isFunction(changed_callback))
    std::range_error("changed_callback must be a function or NULL");

  std::vector<PEventLogger> event_loggers;
  if (!logging.isNull()) {
    List l(logging);
    event_loggers.reserve(l.size());
    for (R_xlen_t i = 0; i < l.size(); ++i) {
      if (!Rf_inherits(l[i], "EventLogger"))
        stop("logging must contain EventLogger objects");
      XP<EventLogger> logger(l[i]);
      event_loggers.push_back(logger);
    }
  }

  if (!contact_rule) {
    if (contact != R_NilValue)
      stop("contact states are required for a contact transition");
    sim->add(new Transition(
      from, to, w, to_change_callback, changed_callback, event_loggers));
  } else {
    if (contact_from.isNull())
      std::range_error("contact from state is NULL");
    if (contact_to.isNull())
      std::range_error("contact to state is NULL");
    List cf(contact_from), ct(contact_to);
    std::optional<std::string> type;
    if (contact != R_NilValue)
      type = contactType(contact);
    sim->add(new ContactTransition(from, cf, to, ct,
        type, w, to_change_callback, changed_callback, event_loggers));
  }
}
