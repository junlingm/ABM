#include "../inst/include/Movement.h"
#include "../inst/include/Simulation.h"

Rcpp::CharacterVector Area::classes = Rcpp::CharacterVector::create("Area", "Contact");

Area::Area(const std::string& state, PMap map, PCollision collision, PMovement movement)
  : _state(state), _map(map), _collision(collision), _movement(movement)
{
  _collision->state = state;
  _movement->state = state;
  _movement->area = this;
  _movement->setDimension(map->dimension());
  _regions.resize(map->regions());
  for (unsigned int i = 0; i < _regions.size(); ++i)
    _regions[i] = new Region(i, *this, map->geometry(i));
}

Area::~Area()
{
  for (auto r : _regions)
    delete r;
}

void Area::add(Agent &agent)
{
  if (_population) {
    Simulation *sim = _population->simulation();
    if (sim) {
      double time =sim->time();
      _movement->init(time, agent);
    }
  }
}

void Area::build()
{
  Simulation *sim = _population->simulation();
  double time = sim->time();
  for (size_t i = 0; i < _population->size(); ++i) {
    auto a = _population->agentAtIndex(i);
    _movement->init(time, *a);
  }
}

void Area::remove(Agent &agent)
{
  throw std::runtime_error("Area::remove not implemented");
}

void Area::migrate(double time, Agent &agent, Region &from)
{
  from.updatePosition(time, agent);
  Rcpp::GenericVector s = agent.state()[_state];
  Rcpp::NumericVector old = Rcpp::as<Rcpp::NumericVector>(s["position"]);
  from.remove(time, agent);
  int i = _map->migrate(from.index(), s["position"], s["velocity"]);
  Region *to = (i < 0) ? nullptr : _regions[i];
  assert(i != from.index());
  _movement->migrate(time, agent, from, to);
  if (!to) to = &from;
  to->add(time, agent);
}

Region::~Region()
{
}

void Region::add(double time, Agent &agent)
{
  _agents.push_back(&agent);
  MovementInfo *info = new MovementInfo(*this);
  store(agent, info);
  schedule(time, agent, true);
}

void Region::remove(double time, Agent &agent)
{
  auto i = std::find(_agents.begin(), _agents.end(), &agent);
  if (i == _agents.end()) return;
  if (i != _agents.end() - 1) *i = _agents.back();
  _agents.pop_back();
  unschedule(time, agent, true);
  erase(agent);
}

void Region::updatePosition(double time, Agent &agent)
{
  Rcpp::NumericVector t(1, time);
  Rcpp::GenericVector m = agent.state()[_area.state()];
  Rcpp::NumericVector p = m["position"];
  Rcpp::NumericVector p0 = p * 1.0;
  Rcpp::NumericVector v = m["velocity"];
  double t0 = Rcpp::as<double>(m["time"]);
  p += v * (time - t0);
  m["position"] = p;
  m["time"] = t;
  Rcpp::GenericVector s;
  s[_area.state()] = m;
  agent.set(s);
}

void Region::update(double time, Agent &agent)
{
  updatePosition(time, agent);
  _area.movement().update(time, agent);
  Rcpp::GenericVector s = agent.state()[_area.state()];
  unschedule(time, agent, true);
  schedule(time, agent, true);
}

void Region::collide(double time, Agent &agent, Agent &with)
{
  updatePosition(time, agent);
  updatePosition(time, with);
  Rcpp::GenericVector sa = agent.state()[_area.state()];
  Rcpp::NumericVector p1 = sa["position"];
  Rcpp::GenericVector sw = with.state()[_area.state()];
  Rcpp::NumericVector p2 = sw["position"];
  unschedule(time, agent, TRUE);
  unschedule(time, with, TRUE);
  auto &movement = _area.movement();
  auto who = movement.collide(time, agent, with);
  bool update = who == Movement::FIRST_ONLY || who == Movement::BOTH;
  schedule(time, agent, update);
  update = who == Movement::SECOND_ONLY || who == Movement::BOTH;
  schedule(time, with, update);
}

void Region::unschedule(double time, Agent &agent, bool update)
{
  MovementInfo *info = storage(agent);
  if (update && info->update) agent.unschedule(info->update);
  if (info->collision) agent.unschedule(info->collision);
  if (info->migrate) agent.unschedule(info->migrate);
  // remove the collision records from the other agents
  for (auto &a : _agents) {
    if (a == &agent) continue;
    for (auto c = info->collisions.begin(); c != info->collisions.end(); ) {
      // if the collision is with the agent
      if (c->second == a) {
        //remove it from info.collisions
        info->collisions.erase(c);
        // loop through the collisions of a to find the one with agent
        MovementInfo *ia = storage(*a);
        for (auto d = ia->collisions.begin(); d != ia->collisions.end(); ++d) {
          if (d->second == &agent) {
            // is this collision the first event of a?
            // if so, unschedule the collision event from a
            if (d == ia->collisions.begin() && ia->collision) {
              a->unschedule(ia->collision);
              ia->collision = nullptr;
            }
            // remove the collision record
            ia->collisions.erase(d);
            // if the collision has been unscheduled, schedule the first
            // event in ia->collisions
            if (!ia->collision && !ia->collisions.empty()) {
              auto first = ia->collisions.begin();
              ia->collision = PEvent(new CollisionEvent(first->first, *first->second, *this));
              a->schedule(ia->collision);
            }
            break;
          }
        }
        // we are done with this agent
        break;
      } else ++c;
    }
  }
}

void Region::schedule(double time, Agent &agent, bool update)
{
  MovementInfo *info = storage(agent);
  // update movement
  if (update) {
    double t = _area.movement().nextUpdateEvent(time, agent);
    if (t < R_PosInf) {
      info->update = PEvent(new UpdateMovement(t, *this));
      agent.schedule(info->update);
    }
  }
  const Collision &collision = _area.collision();
  // detect new collisions
  for (auto &with : _agents) {
    if (with != &agent) {
      MovementInfo *iw = storage(*with);
      double t = collision.time(time, agent, *with);
      if (0 < t && t < R_PosInf) {
        info->collisions[t] = with;
        // here we maintain the collision at agent, not at with because
        // we only schedule one collision event per collision
      }
    }
  }
  if (!info->collisions.empty()) {
    auto first = info->collisions.begin();
    info->collision = PEvent(new CollisionEvent(first->first, *first->second, *this));
    agent.schedule(info->collision);
  } else info->collision = nullptr;
  // update collision with the boundaries
  if (info->migrate) agent.unschedule(info->migrate);
  Rcpp::GenericVector s = agent.state()[_area.state()];
  double t = _geometry.hitBoundary(time, s["position"], s["velocity"]);
  if (time < t && t < R_PosInf) {
    info->migrate = PEvent(new MigrationEvent(t, *this));
    agent.schedule(info->migrate);
  } else info->migrate = nullptr;
}

bool CollisionEvent::handle(Simulation &sim, Agent &agent)
{
  _region.collide(time(), agent, _with);
  _region.area().collision().handle(time(), sim, agent, _with);
  // discard the event
  return false;
}

bool MigrationEvent::handle(Simulation &sim, Agent &agent)
{
  _region.area().migrate(time(), agent, _region);
  // discard the event
  return false;
}

Collision::~Collision()
{
}

Rcpp::CharacterVector Collision::classes = Rcpp::CharacterVector::create("Collision");

double RCollision::time(double time, Agent &agent, Agent &with) const
{
  Rcpp::NumericVector t = _calculator(Rcpp::wrap(time), XP<Agent>(agent), XP<Agent>(with));
  return t[0];
}

double RadiusCollision::time(double time, Agent &agent, Agent &with) const
{
  Rcpp::GenericVector sa = agent.state()[state];
  double ta = Rcpp::as<double>(sa["time"]);
  Rcpp::NumericVector pa = sa["position"];
  Rcpp::NumericVector va = sa["velocity"];
  Rcpp::NumericVector a = pa + va * (time - ta);
  Rcpp::GenericVector sw = with.state()[state];
  double tw = Rcpp::as<double>(sw["time"]);
  Rcpp::NumericVector pw = sw["position"];
  Rcpp::NumericVector vw = sw["velocity"];
  Rcpp::NumericVector w = pw + vw * (time - tw);
  Rcpp::NumericVector dp = a - w;
  Rcpp::NumericVector dv = va - vw;
  double p2 = sum(dp * dp);
  double v2 = sum(dv * dv);
  // if they are parallel, they will collide if ||p1 - p2|| < radius
  if (v2 == 0) return R_PosInf;
  double vp = sum(dp * dv);
  double Delta = vp * vp - v2 * (p2 - _r2);
  if (Delta < 0) return R_PosInf;
  Delta = sqrt(Delta);
  // there are two possible time values where the two agents are up to the 
  // distance of the radius: one entering the distance, one leaving it
  // If both time are negative, the agents have already collided. If one is
  // negative (or zero) and the other is positive, they are colliding.
  // If both are positive, the smaller time is the time entering collision.
  double dt = (-vp - Delta) / v2;
  if (dt <= 0) dt = (-vp + Delta) / v2;
  return (dt <= 0) ? R_PosInf : dt + time;
}

Movement::~Movement()
{
}

Rcpp::CharacterVector Movement::classes = Rcpp::CharacterVector::create("Movement");

RandomWalk::RandomWalk(double speed, double rate)
  : _speed(speed), _unif(0, 1), _exp(rate) 
{
}

void RandomWalk::setDimension(unsigned int dimension)
{
  if (dimension < 1 || dimension > 3)
    throw std::invalid_argument("dimension must be 1, 2 or 3");
  _dimension = dimension;
}

Rcpp::NumericVector RandomWalk::randomVelocity()
{
  Rcpp::NumericVector v(_dimension);
  double theta = _unif.get();
  switch (v.size()) {
  case 1:
    v[0] = theta > 0.5 ? -_speed : _speed;
    break;
  case 2:
    theta *= 2 * M_PI;
    v[0] = _speed * cos(theta);
    v[1] = _speed * sin(theta);
    break;
  case 3: {
      theta *= 2 * M_PI;
      double phi = _unif.get() * M_PI;
      v[0] = _speed * sin(phi) * cos(theta);
      v[1] = _speed * sin(phi) * sin(theta);
      v[2] = _speed * cos(phi);
      break;
    }
  default:
    throw std::runtime_error("I do not know how to update a random walk in dimensions other than 1, 2, 3.");
  }
  return v;
}

void RandomWalk::update(double time, Agent &agent)
{
  Rcpp::GenericVector s = agent.state()[state];
  s["time"] = time;
  s["velocity"] = randomVelocity();
}

double RandomWalk::nextUpdateEvent(double time, Agent &agent)
{
  return time + _exp.get();
}

Movement::CollisionType RandomWalk::collide(double time, Agent &agent, Agent &with)
{
  Rcpp::GenericVector s = agent.state()[state];
  Rcpp::GenericVector sw = with.state()[state];
  Rcpp::NumericVector v = s["velocity"];
  Rcpp::NumericVector vw = sw["velocity"];
  s["velocity"] = sw["velocity"];
  sw["velocity"] = v;
  return BOTH;
}

void RandomWalk::migrate(double time, Agent &agent, Region &from, Region *to)
{
  if (!to) {
    Rcpp::GenericVector s = agent.state()[state];
    Rcpp::NumericVector v = s["velocity"];
    s["time"] = time;
    s["velocity"] = -v;
  }
}

void RandomWalk::init(double time, Agent &agent)
{
  auto pos = area->map().randomPosition();
  auto i = area->map().region(pos);
  Rcpp::GenericVector m;
  m["time"] = Rcpp::wrap(time);
  m["position"] = pos;
  m["velocity"] = randomVelocity();
  Rcpp::GenericVector s;
  s[area->state()] = m;
  agent.set(s);
  area->region(i).add(time, agent);
}

bool UpdateMovement::handle(Simulation &sim, Agent &agent)
{
  _region.update(time(), agent);
  return false;
}

//---------------
// R interface
//---------------

// [[Rcpp::export]]
/**
 * Create an RCollision object
 * @param calculator function to calculate the time of collision
 * @param handler function to call when collision occurs
 */
XP<Collision> newRCollision(Rcpp::Function calculator, Rcpp::Function handler)
{
  PCollision p(new RCollision(calculator, handler));
  return XP<Collision>(p);
}

// [[Rcpp::export]]
/**
 * Create a RadiusCollision object
 * @param radius radius of the collision
 * @param handler function to call when collision occurs
 */
XP<Collision> newRadiusCollision(double radius, Rcpp::Function handler, std::string state)
{
  PCollision p(new RadiusCollision(radius, handler));
  p->state = state;
  return XP<Collision>(p);
}

// [[Rcpp::export]]
/**
 * check collision between two agents
 * @param c collision object
 * @param agent first agent
 * @param with second agent
 */
double collision(double time, XP<Collision> c, XP<Agent> agent, XP<Agent> with)
{
  return c->time(time, **agent, **with);
}

// [[Rcpp::export]]
/**
 * Create a new random walk movement
 * @param dimension dimension of the movement
 * @param speed speed of the movement
 * @param rate rate of the exponential distribution of the time between updates
 */
XP<Movement> newRandomWalk(double speed, double rate)
{
  PMovement p(new RandomWalk(speed, rate));
  return XP<Movement>(p);
}

// [[Rcpp::export]]
/**
 * Create a new area
 * @param state name of the state
 * @param map map of the area
 * @param collision collision object
 */
XP<Area> newArea(const std::string& state, XP<Map> map, XP<Collision> collision, 
                 XP<Movement> movement)
{
  PArea p(new Area(state, map, collision, movement));
  return XP<Area>(p);
}
