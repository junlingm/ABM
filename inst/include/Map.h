#pragma once

#include "XP.h"
#include "RNG.h"

/**
 * provide a geometry for a region
 * 
 * A geometry provides two functionalities, checking if it contains a point,
 * and calculating the time that a movement hits a boundary.
 */
class Geometry {
public:
  virtual ~Geometry() {}
  virtual bool contains(const Rcpp::NumericVector& point) const = 0;
  virtual double hitBoundary(double time, const Rcpp::NumericVector &position,
                             const Rcpp::NumericVector &velocity) const = 0;
};

/**
 * provide a map for an area
 * 
 * A map contains a list of geometry, each corresponding to a region
 */
class Map {
public:
  /**
   * create a map
   * @param dimension the dimension of the map
   * @param size the number of regions in the map
   */
  Map(unsigned int dimension, unsigned int size) 
  : _dimension(dimension), _geometries(size) {}
  virtual ~Map();
  /**
   * return the dimension of the map
   */
  unsigned int dimension() const { return _dimension; }
  /**
   * return the number of regions in the map
   */
  unsigned int regions() const { return _geometries.size(); }
  /**
   * return the geometry of a region
   * @param index the index of the region
   * @return the geometry of the region
   */
  const Geometry& geometry(unsigned int index) const { return *_geometries[index]; }
  /**
   * return the index of the region that contains a point
   * @param point the point to check
   * @return the index of the region that contains the point, or -1 if no region
   * contains the point
   */
  virtual int region(const Rcpp::NumericVector& point) const = 0;
  /**
   * returns the region that a movement will migrate to after hitting a boundary
   * @param from the region that the movement is migrating from
   * @param position the position of the movement
   * @param velocity the velocity of the movement
   * @return the region that the movement will migrate to, or -1 if the movement
   * will not migrate
   */
  virtual int migrate(unsigned int from,
                      const Rcpp::NumericVector &position,
                      const Rcpp::NumericVector &velocity) const = 0;
  
  /** 
   * return a random position, and the region that this position is in 
   */
  virtual Rcpp::NumericVector randomPosition() const = 0;
  
  
  static Rcpp::CharacterVector classes;
  
protected:
  unsigned int _dimension;
  std::vector<Geometry*> _geometries;
};

typedef std::shared_ptr<Map> PMap;

/**
 * A box
 */
class Box : public Geometry {
public:
  Box(const Rcpp::NumericVector& lower, const Rcpp::NumericVector& upper)
    : _lower(lower), _upper(upper) {}
  
  virtual bool contains(const Rcpp::NumericVector& point) const;
  virtual double hitBoundary(double time, const Rcpp::NumericVector &position,
                             const Rcpp::NumericVector &velocity) const;
  const Rcpp::NumericVector& lower() const { return _lower; }
  const Rcpp::NumericVector& upper() const { return _upper; }
  
protected:
  Rcpp::NumericVector _lower;
  Rcpp::NumericVector _upper;
};

/**
 * Lattice is a map that contains a list of boxes
 */
class Lattice : public Map {
public:
  /**
   * create a lattice
   * @param lower the lower bound of the lattice
   * @param upper the upper bound of the lattice
   * @param divisions the number of divisions in each dimension
   * @param toroidal whether the lattice is toroidal
   */
  Lattice(const Rcpp::NumericVector& lower, const Rcpp::NumericVector& upper, 
          const Rcpp::IntegerVector& divisions, bool toroidal = false);

  virtual int region(const Rcpp::NumericVector& point) const;
  virtual int migrate(unsigned int from,
                      const Rcpp::NumericVector &position,
                      const Rcpp::NumericVector &velocity) const;
  virtual Rcpp::NumericVector randomPosition() const;
  
protected:
  static unsigned int size(const Rcpp::IntegerVector& divisions);
  Rcpp::NumericVector _lower;
  Rcpp::NumericVector _upper;
  Rcpp::IntegerVector _divisions;
  bool _toroidal;
  Rcpp::NumericVector _unit;
  std::vector<int> _dim;
};

