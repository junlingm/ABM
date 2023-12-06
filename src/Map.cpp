#include "../inst/include/Map.h"
#include <cmath>

double epsilon = 1e-8;

Rcpp::CharacterVector Map::classes = Rcpp::CharacterVector::create("Map");

bool Box::contains(const Rcpp::NumericVector& point) const
{
  for (int i = 0; i < point.size(); ++i) {
    double diff = _upper[i] - _lower[i];
    if (point[i] - _lower[i] < - epsilon * diff  || point[i] > _upper[i] + epsilon * diff)
      return false;
  }
  return true;
}

std::pair<double, Rcpp::NumericVector> Box::hitBoundary(
    double time, const Rcpp::NumericVector &position,
    const Rcpp::NumericVector &velocity) const
{
  double hitTime = R_PosInf, t;
  int border;
  double dir;
  Rcpp::NumericVector normal(position.size(), 0);
  bool hit;
  for (int i = 0; i < position.size(); ++i) {
    if (velocity[i] > 0) {
      if (_upper[i] < position[i]) {
        hit = true;
        t = 0;
      } else {
        t = (_upper[i] - position[i]) / velocity[i];
        hit = t > -epsilon && hitTime > t;
      }
      if (hit) {
        border = i;
        dir = 1;
        hitTime = t;
      }
    } else if (velocity[i] < 0) {
      if (_lower[i] > position[i]) {
        hit = true;
        t = 0;
      } else {
        t = (_lower[i] - position[i]) / velocity[i];
        hit = t > -epsilon && hitTime > t;
      }
      if (hit) {
        border = i;
        dir = -1;
        hitTime = t;
      }
    }
  }
  normal[border] = dir;
  return std::make_pair(hitTime + time, normal);
}

Map::~Map()
{
  for (size_t i = 0; i < _geometries.size(); ++i)
    delete _geometries[i];
}

unsigned int Lattice::size(const Rcpp::IntegerVector& divisions)
{
  unsigned int size = 1;
  for (int i = 0; i < divisions.size(); ++i)
  {
    size *= divisions[i];
  }
  return size;
}

Lattice::Lattice(const Rcpp::NumericVector& lower, const Rcpp::NumericVector& upper, 
                 const Rcpp::IntegerVector& divisions, bool toroidal)
  : Map(lower.size(), size(divisions)), 
    _lower(lower), _upper(upper), _divisions(divisions), _toroidal(toroidal), 
    _unit(lower.size()), _dim(lower.size(), 1)
{
  for (unsigned int i = 1; i < _dimension; ++i)
    _dim[i] = _dim[i-1] * divisions[i];
  for (unsigned int i = 0; i < _dimension; ++i)
    _unit[i] = (upper[i] - lower[i]) / divisions[i];
  std::vector<int> d(_dimension, 0);
  size_t n = _geometries.size(), k = 0;
  for (size_t i = 0; i < n; ++i) {
    Rcpp::NumericVector l(_dimension), u;
    for (unsigned int j = 0; j < _dimension; ++j) {
      l[j] = _unit[j] * d[j] + lower[j];
    }
    u = l + _unit;
    _geometries[i] = new Box(l, u);
    if (++d[k] == _divisions[k]) {
      d[k] = 0;
      if (k < _dimension - 1) ++d[k+1];
      k = 0;
    }
  }
}

int Lattice::region(const Rcpp::NumericVector& point) const
{
  unsigned int index = 0;
  for (unsigned int i = 0; i < _dimension; ++i) {
    unsigned int d = static_cast<unsigned int>(point[i] / _unit[i]);
    index += d * _dim[i];
  }
  return index;
}

Rcpp::NumericVector Lattice::randomPosition() const
{
  Rcpp::NumericVector u = Rcpp::runif(_dimension);
  return _lower + (_upper - _lower) * u;
}

int Lattice::migrate(unsigned int from, Rcpp::NumericVector &position, 
                     const Rcpp::NumericVector &normal) const
{
  // calculate the normal vector of the boundary at position
  std::vector<int> dir(_dimension);
  for (unsigned int i = 0; i < _dimension; ++i) {
    int d;
    if (normal[i] > epsilon) 
      d = 1;
    else if (normal[i] < -epsilon)
      d = -1;
    else
      d = 0;
    dir[i] = (from / _dim[i]) % _divisions[i] + d;
    if (dir[i] < 0) {
      if (!_toroidal) return -1;
      dir[i] = _divisions[i] - 1;
      position[i] = _upper[i];
    } else if (dir[i] >= _divisions[i]) {
      if (!_toroidal) return -1;
      position[i] = _lower[i];
      dir[i] = 0;
    }
  }
  unsigned int to = 0;
  for (unsigned int i = 0; i < _dimension; ++i) {
    to += dir[i] * _dim[i];
  }
  return to;
}

//------------------------------
// R interface
//------------------------------

// [[Rcpp::export]]
XP<Map> newLattice(const Rcpp::NumericVector& lower, const Rcpp::NumericVector& upper, 
                            const Rcpp::IntegerVector& divisions, bool toroidal)
{
  PMap map(new Lattice(lower, upper, divisions, toroidal));
  return XP<Map>(map);
}

// [[Rcpp::export]]
unsigned int regionAtPoint(const XP<Map> &map, const Rcpp::NumericVector& point)
{
  return map->region(point);
}

// [[Rcpp::export]]
Rcpp::NumericVector randomPosition(const XP<Map> &map)
{
  return map->randomPosition();
}
