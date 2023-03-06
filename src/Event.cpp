#include "../inst/include/Simulation.h"

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

// [[Rcpp::export]]
XP<Event> newEvent(double time, Function handler)
{
  return XP<Event>(std::make_shared<REvent>(time, handler));
}
  
// [[Rcpp::export]]
double getTime(XP<Event> event)
{
  return event->time();
}

CharacterVector Event::classes = CharacterVector::create("Event");
