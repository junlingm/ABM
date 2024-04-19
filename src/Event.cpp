#include "../inst/include/Simulation.h"
#include <cmath>

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

Calendar::Calendar()
  : Event(R_PosInf)
{
}

bool Calendar::handle(Simulation &sim, Agent &agent)
{
  if (!_events.empty()) {
    auto e = _events.begin()->second;
    unschedule(e);
    if (e->handle(sim, agent)) {
      schedule(e);
    }
  }
  return true;
}

void Calendar::schedule(PEvent event)
{
  if (event->_owner != nullptr)
    event->_owner->unschedule(event);
  double t = event->time();
  bool update = _time > t;
  if (update) _time = t;
  Calendar *owner = update ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  event->_owner = this;
  event->_pos = _events.emplace(t, event);
  if (owner != nullptr)
    owner->schedule(me);
}

void Calendar::unschedule(PEvent event)
{
  if (event == NULL || event->_owner != this) return;
  Calendar *owner = (_time == event->time()) ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  _events.erase(event->_pos);
  event->_owner = nullptr;
  _time = _events.empty() ? R_PosInf : _events.begin()->first;
  if (owner != nullptr)
    owner->schedule(me);
}

void Calendar::clearEvents()
{
  Calendar *owner = !std::isinf(_time) ? _owner : nullptr;
  PEvent me;
  if (owner != nullptr) {
    me = _pos->second;
    owner->unschedule(me);
  }
  for (auto e : _events)
    e.second->_owner = NULL;
  _events.clear();
  _time = R_PosInf;
  if (owner != nullptr)
    owner->schedule(me);
}
