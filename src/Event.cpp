#include "Event.h"
#include "Agent.h"
#include "Simulation.h"

using namespace Rcpp;

Event::Event(double time)
  : _owner(nullptr), _time(time)
{
}

Event::~Event()
{
}

REvent::REvent(double time, Function handler)
  : Event(time), _handler(handler)
{
}

bool REvent::handle(Simulation &sim, Agent &agent)
{
  _handler(wrap(time()), XP<Simulation>(sim), XP<Agent>(agent));
  return false;
}

extern "C" {
  SEXP newEvent(SEXP time, SEXP handler)
  {
    Function h(handler);
    return XP<Event>(std::make_shared<REvent>(as<double>(time), h));
  }
  
  SEXP getTime(SEXP event)
  {
    XP<Event> e(event);
    return wrap(e->time());
  }
}

CharacterVector Event::classes = CharacterVector::create("Event");
