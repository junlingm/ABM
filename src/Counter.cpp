#include "../inst/include/Counter.h"

using namespace Rcpp;

Logger::Logger(const std::string &name)
  : _name(name)
{
}

Logger::~Logger()
{
}

Counter::Counter(const std::string &name, const List &state, Nullable<List> to, long initial)
  : Logger(name), _count(initial), _state(state), _to(to)
{
}

void Counter::log(const Agent &agent, const State &from_state)
{
  if (_to.isNull()) {
    std::string s = as<std::string>(_state[0]);
    if (from_state.match(_state)) {
      --_count;
    }
    if (agent.match(_state)) {
      ++_count;
    }
  } else if (agent.match(_to.as()) && from_state.match(_state))
    ++_count;
}

long Counter::report()
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

long StateLogger::report()
{
  return _value;
}

// [[Rcpp::export]]
XP<Counter> newCounter(std::string name, List from, Nullable<List> to=R_NilValue, int initial=0)
{
  return XP<Counter>(std::make_shared<Counter>(name, from, to, initial));
}

// [[Rcpp::export]]
XP<StateLogger> newStateLogger(std::string name, Nullable<XP<Agent> > agent, std::string state)
{
  PAgent pa;
  if (agent.isNotNull()) pa = agent.as();
  return std::make_shared<StateLogger>(name, pa, state);
}

Rcpp::CharacterVector Counter::classes = CharacterVector::create("Counter", "Logger");
Rcpp::CharacterVector StateLogger::classes = CharacterVector::create("StateLogger", "Logger");
