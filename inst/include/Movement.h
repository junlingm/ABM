#pragma once

#include "Contact.h"
#include "Map.h"
#include "RNG.h"

class Region;
/**
 * CollisonEvent is an REvent class to detect the collisions of an agent with others.
 */
class CollisionEvent : public Event {
public:
  CollisionEvent(double time, Agent &with, Region &region)
    : Event(time), _with(with), _region(region) { }
  
  Agent &with() { return _with; }
  const Agent &with() const { return _with; }
  
protected:
  virtual bool handle(Simulation &sim, Agent &agent);
  Agent &_with;
  Region &_region;
};

/**
 * An internal event for an agent to migrate.
 * 
 * This event is caused by an agent hitting the boundary of the region
 */
class MigrationEvent : public Event {
public:
  /**
   * Constructor
   * @param time the time of the event
   * @param region the region that the agent migrates from
   * @param normal the normal vector of the boundary that the agent hits
   */
  MigrationEvent(double time, Region &region, Rcpp::NumericVector normal)
    : Event(time), _region(region), _normal(normal) { }
  
  Region &region() { return _region; }
  Rcpp::NumericVector normal() { return _normal; }
  
protected:
  virtual bool handle(Simulation &sim, Agent &agent);
  Region &_region;
  Rcpp::NumericVector _normal;
};

/**
 * An internal event for updating the movement of an agent.
 * 
 * This event is caused by an agent changing its movement
 */
class UpdateMovement : public Event {
public:
  UpdateMovement(double time, Region &region) : Event(time), _region(region) { }
  
protected:
  virtual bool handle(Simulation &sim, Agent &agent);
  Region &_region;
};

/**
 * A struct to store an agent movement information in a region.
 * 
 * The information include
 * - the region the agent is in
 * - a list of collisions with other agents
 * - a collision with the boundary
 * - the next update movement event
 */
struct MovementInfo {
  MovementInfo(Region &r) : region(r) { }
  Region &region;
  /**
   * a map of collisions with other agents. The key is the time of the collision
   */
  std::map<double, Agent *> collisions;
  PEvent collision;
  PEvent migrate;
  PEvent update;
};

class Area;
/**
 * A region is a subregion of the area. It is a subclass of Calendar to manage
 * the collision events in the region. It is used to speed up the contact
 * detection, by only checking agents in the same region. It maintains a list of
 * agents that are in the region. It has a virtual method to check if a point is
 * in the region.
 */
class Region : public Storage<MovementInfo> {
public:
  /**
   * Constructor
   * @param index the index of the region
   * @param area the area that the region belongs to
   * @param geometry the geometry of the region
   */
  Region(unsigned int index, Area &area, const Geometry &geometry) 
  : Storage<MovementInfo>("Region"), _index(index), _area(area), _geometry(geometry) { }
  
  virtual ~Region();
  
  /** the area that this region is in */
  Area &area() { return _area; }
  /** the geometry of the region */
  const Geometry &geometry() { return _geometry; }
  /** the index of the region */
  unsigned int index() const { return _index; }

  /**
   * Add an agent to the region
   * @param time the current time
   * @param agent the agent to add
   */
  virtual void add(double time, Agent& agent);
  /**
   * Remove an agent from the region
   * @param time the current time
   * @param agent the agent to remove
   */
  virtual void remove(double time, Agent &agent);
  /**
   * handle the UpdateMovement event
   * @param time the current time
   * @param agent the agent to update
   */
  virtual void update(double time, Agent &agent);
  /**
   * The CollisionEvent handler
   * @param time the current time
   * @param agent the agent to update
   * @param collided the agent that the agent collides with. 
   */
  virtual void collide(double time, Agent &agent, Agent &with);
  /**
   * update the position of an agent with current time
   */
  void updatePosition(double time, Agent &agent);
  
protected:
  /**
   * unschedule all movement related events of an agent
   * @param time the current time
   * @param agent the agent to update
   * @param update whether to update the movement of the agent
   */
  void unschedule(double time, Agent &agent, bool update);
  /**
   * schedule all movement events of an agent
   * @param time the current time
   * @param agent the agent to update
   * @param update whether to update the movement of the agent
   */
  void schedule(double time, Agent &agent, bool update);
  /** the index of the region */
  unsigned int _index;
  /**
   * the area that the region belongs to
   */
  Area &_area;
  /**
   * the geometry of the region
   */
  const Geometry &_geometry;
  /**
   * the list of agents in the region
   */
  std::vector<Agent*> _agents;
};

class Collision {
public:
  Collision(Rcpp::Nullable<Rcpp::Function> handler) : _handler(handler) { }
  virtual ~Collision();
  /** the time that the agent hits another agent
   * @param agent the agent to check
   * @param with the other agent to check
   * @return a time or R_PosInf if the agent never hits the other agent
   */
  virtual double time(double time, Agent &agent, Agent &with) const = 0;
  
  /** the handler of a collision event */
  void handle(double time, Simulation &sim, Agent &agent, Agent &with) { 
    if (!_handler.isNull()) 
      _handler.as()(time, XP<Simulation>(sim), XP<Agent>(agent), XP<Agent>(with)); }
  
  /** required by the XP class. */
  static Rcpp::CharacterVector classes;
  /** the state of agents that holds the movement information */
  std::string state;
  
protected:
  /** the Collision event handler */
  Rcpp::Nullable<Rcpp::Function> _handler;
};

typedef std::shared_ptr<Collision> PCollision;

class RCollision : public Collision {
public:
  RCollision(Rcpp::Function calculator, Rcpp::Nullable<Rcpp::Function> handler) 
    : Collision(handler), _calculator(calculator) { }
  virtual double time(double time, Agent &agent, Agent &with) const;
  
private:
  Rcpp::Function _calculator;
};

class RadiusCollision : public Collision {
public:
  RadiusCollision(double radius, Rcpp::Nullable<Rcpp::Function> handler) 
    : Collision(handler), _r2(radius * radius) { }
  virtual double time(double time, Agent &agent, Agent &with) const;
  
private:
  /**
   * the squared radius of the collision
   */
  double _r2;
};

/**
 * A class that defines the movement of agents.
 * 
 * An area has a movement object to define the movement of agents.
 * When an agent's movement should change, e.g., when collision happens, or it
 * enters a new region, or other reasons like voluntarily change of movement,
 * the agent will notice the area, and the area will call the movement object to
 * update the movement of the agent.
 */
class Movement {
public:
  typedef enum {
    NEITHER, /** neither agent needs to be updated */
    FIRST_ONLY, /** only the first agent needs to be updated */  
    SECOND_ONLY, /** only the second agent needs to be updated */  
    BOTH, /** both agents need to be updated */ 
  } CollisionType;
  
  virtual ~Movement();
  
  /**
   * initialize the movement information of an agent
   * @param time the current time
   * @param agent the agent to update
   */
  virtual void init(double time, Agent &agent) = 0;
  
  /**
   * migrate an agent from one region to another
   * @param time the current time
   * @param agent the agent to update
   * @param from the region that the agent is migrating from
   * @param to the region that the agent is migrating to. 
   * @param normal the normal vector of the boundary where the agent hits in 
   * the from region.
   * @return the time that the agent will change its movement. 
   * @details If to == NULL, then the agent hits a boundary of a region, and 
   * there is no region beyond the boundary.
   * 
   * If the value is R_PosInf, then the agent does not change its movement.
   */
  virtual void migrate(double time, Agent &agent, Region &from, Region *to,
                       Rcpp::NumericVector normal) = 0;
  
  /**
   * update the movement of an agent after a collision
   * @param time the current time
   * @param agent the agent to update
   * @param collided the agent that the agent collides with.
   * @return which agent's movement need to be updated
   */
  virtual CollisionType collide(double time, Agent &agent, Agent &collided) = 0;
  
  /**
   * update the movement of an agent at the end of a segment of movement
   * @param time the current time
   * @param agent the agent to update
   * @return the time that the agent will change its movement
   */
  virtual void update(double time, Agent &agent) = 0;
  /**
   * return the next update event time
   * @param time the current time
   * @param agent the agent to update
   * @return the time that the agent will change its movement
   */
  virtual double nextUpdateEvent(double time, Agent &agent) = 0;
  /**
   * set the dimension of the movement
   * @param dimension the dimension of the movement
   */
  virtual void setDimension(unsigned int dimension) { _dimension = dimension; }
  unsigned int dimension() const { return _dimension; }

  /** required by the XP class. */
  static Rcpp::CharacterVector classes;
  /** the state of agents that holds the movement information */
  std::string state;
  /** the area that is associated with this movement */
  Area *area;
  
protected:
  /** the dimension of the movement */
  unsigned int _dimension;
};

typedef std::shared_ptr<Movement> PMovement;

/**
 * A random walk movement.
 */
class RandomWalk : public Movement {
public:
  /**
   * Constructor
   * @param speed the speed of the random walk
   * @param rate the rate of direction change of the random walk
   */
  RandomWalk(double speed, double rate);
  
  virtual void init(double time, Agent &agent);
  virtual void migrate(double time, Agent &agent, Region &from, Region *to,
                       Rcpp::NumericVector normal);
  virtual CollisionType collide(double time, Agent &agent, Agent &collided);
  virtual void update(double time, Agent &agent);
  virtual double nextUpdateEvent(double time, Agent &agent);
  virtual void setDimension(unsigned int dimension);

protected:
  /** generate a random velocity */
  Rcpp::NumericVector randomVelocity();
  
private:
  double _speed;
  RUnif _unif;
  RExp _exp;
};

/**
 * Area is a subclass of Contact to describe agent movement. it is divided into
 * regions, agents are assigned to regions according to their coordinates.
 */
class Area : public Contact {
public:
  /**
   * Constructor
   * 
   * @param state the state of an agent that contains the movement information
   * @param map the map object to calculate the region of an agent
   * @param collision the collision object to calculate collision time
   * @param movement the movement object to update agent movement
   * @details the state contains 3 components
   * - time: time that the state was set
   * - position: the position of an agent
   * - velocity: the velocity of an agent
   */
  Area(const std::string& state, PMap map, PCollision collision, PMovement movement);
  virtual ~Area();
  
  /**
   * Add an agent to the region
   * @param time the current time
   * @param agent the agent to add
   */
  virtual void add(Agent &agent);
  /**
   * the agent will migrate to another region
   * @param time the current time
   * @param agent the agent to migrate
   */
  virtual void remove(Agent &agent);
  /**
   * get called if an agent's movement has changed in the region
   * @param time the current time
   * @param agent the agent whose movement has changed
   * @param from the region that the agent is migrating from
   * @param normal the normal vector of the boundary where the agent hits in
   * the from region
   */
  virtual void migrate(double time, Agent& agent, Region &from, Rcpp::NumericVector normal);
  
  const std::string &state() const { return _state; }
  
  Collision &collision() { return *_collision; }
  
  Movement &movement() { return *_movement; }
  
  Map &map() { return *_map; }
  
  Region &region(unsigned int i) { return *_regions[i]; }
  
  static Rcpp::CharacterVector classes;

  const std::vector<Agent*> &contact(double time, Agent &agent) { return _contacts; }
  
protected:
  /**
   * build the area. This is called when the Area is attached to th simulation
   */
  virtual void build();
  
  /**
   * the state of an agent that contains the movement information
   */
  std::string _state;
  /** the map object */
  PMap _map;
  /** the collision detector */
  PCollision _collision;
  /** the movement object */
  PMovement _movement;
  /**
   * the regions of the area
   */
  std::vector<Region*> _regions;
  std::vector<Agent*> _contacts;
};

typedef std::shared_ptr<Area> PArea;
