#include "../inst/include/Counter.h"

using namespace Rcpp;

Logger::Logger(const std::string &name)
  : _name(name)
{
}

Logger::~Logger()
{
}

bool Logger::stateChanging(const Agent &agent, const List &state)
{
  return false;
}

void Logger::stateChanged(const Agent &agent)
{
}

Counter::Counter(const std::string &name, const List &state, Nullable<List> to, long initial)
  : Logger(name), _count(initial), _from_match(false), _state(state), _to(to)
{
}

void Counter::log(const Agent &agent, const State &from_state)
{
  if (_to.isNull()) {
    if (from_state.match(_state)) {
      --_count;
    }
    if (agent.match(_state)) {
      ++_count;
    }
  } else if (agent.match(List(_to)) && from_state.match(_state))
    ++_count;
}

bool Counter::stateChanging(const Agent &agent, const List &state)
{
  _from_match = agent.match(_state);
  // An occupancy counter can enter or leave its state, so it must always
  // be considered. A transition counter can only count when its source
  // state matched before the change.
  return _to.isNull() || _from_match;
}

void Counter::stateChanged(const Agent &agent)
{
  if (_to.isNull()) {
    if (_from_match) --_count;
    if (agent.match(_state)) ++_count;
  } else if (_from_match && agent.match(List(_to))) {
    ++_count;
  }
  _from_match = false;
}

double Counter::report()
{
  long x = _count;
  if (!_to.isNull()) _count = 0;
  return x;
}

StateLogger::StateLogger(const std::string &name, PAgent agent, const std::string &state)
  : Logger(name), _value(R_NaN), _agent(agent), _state(state)
{
}

void StateLogger::log(const Agent &agent, const State &from_state)
{
  PAgent pa = _agent.lock();
  const Agent &a = pa ? *pa : agent;
  _value = as<double>(a.state()[_state]);
}

bool StateLogger::stateChanging(const Agent &agent, const List &state)
{
  return _agent.expired();
}

void StateLogger::stateChanged(const Agent &agent)
{
  if (_agent.expired())
    _value = as<double>(agent.state()[_state]);
}

double StateLogger::report()
{
  PAgent pa = _agent.lock();
  if (pa)
    return as<double>(pa->state()[_state]);
  return _value;
}

// [[Rcpp::export]]
XP<Counter> newCounter(std::string name, List from, Nullable<List> to=R_NilValue, int initial=0)
{
  warning("newCounter() is deprecated; use newStateLogger() with inc() or dec() instead");
  return XP<Counter>(std::make_shared<Counter>(name, from, to, initial));
}

// [[Rcpp::export]]
XP<StateLogger> newStateLogger(std::string name, Nullable<XP<Agent> > agent, std::string state)
{
  PAgent pa;
  if (agent.isNotNull()) pa = XP<Agent>(agent);
  return std::make_shared<StateLogger>(name, pa, state);
}

Rcpp::CharacterVector Counter::classes = CharacterVector::create("Counter", "Logger");
Rcpp::CharacterVector StateLogger::classes = CharacterVector::create("StateLogger", "Logger");
