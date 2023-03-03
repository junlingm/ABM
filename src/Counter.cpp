#include "Counter.h"

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
  const Agent &a = (_agent) ? *_agent : agent;
  _value = as<double>(a.state()[_state]);
}

long StateLogger::report()
{
  return _value;
}

extern "C" {
  SEXP newCounter(SEXP name, SEXP from, SEXP to, SEXP initial)
  {
    std::string n = as<std::string>(name);
    List f(from);
    Nullable<List> t(to);
    int i0 = as<int>(initial);
    return XP<Counter>(std::make_shared<Counter>(n, f, t, i0));
  }

  SEXP newStateLogger(SEXP name, SEXP agent, SEXP state)
  {
    std::shared_ptr<Agent> pa;
    if (agent != R_NilValue) {
      XP<Agent> a(agent);
      pa = *a;
    }
    std::string n = as<std::string>(name);
    std::string s = as<std::string>(state);
    return XP<StateLogger>(std::make_shared<StateLogger>(n, pa, s));
  }
}

Rcpp::CharacterVector Counter::classes = CharacterVector::create("Counter", "Logger");
Rcpp::CharacterVector StateLogger::classes = CharacterVector::create("StateLogger", "Logger");
