#include "../inst/include/Map.h"
#include <cmath>

static double epsilon = 1e-8;

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

double Box::hitBoundary(double time, const Rcpp::NumericVector &position,
                        const Rcpp::NumericVector &velocity) const
{
  double hitTime = R_PosInf;
  for (int i = 0; i < position.size(); ++i) {
    if (velocity[i] > epsilon)
      hitTime = std::min(hitTime, (_upper[i] - position[i]) / velocity[i]);
    else if (velocity[i] < epsilon)
      hitTime = std::min(hitTime, (_lower[i] - position[i]) / velocity[i]);
  }
  return (hitTime <= epsilon) ? R_PosInf : hitTime + time;
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

int Lattice::migrate(unsigned int from, const Rcpp::NumericVector &position, 
                     const Rcpp::NumericVector &velocity) const
{
  // calculate the normal vector of the boundary at position
  std::vector<int> dir(_dimension);
  Box *b = (Box*)_geometries[from];
  const Rcpp::NumericVector &lower = b->lower();
  const Rcpp::NumericVector &upper = b->upper();
  for (unsigned int i = 0; i < _dimension; ++i) {
    dir[i] = (from / _dim[i]) % _divisions[i];
    if (fabs(position[i] - lower[i]) <= _unit[i] * epsilon) {
      dir[i] -= 1;
      if (dir[i] < 0) {
        if (!_toroidal) return -1;
        dir[i] = _divisions[i] - 1;
      }
    } else if (fabs(position[i] - upper[i]) <= _unit[i] * epsilon) {
      dir[i] += 1;
      if (dir[i] >= _divisions[i]) {
        if (!_toroidal) return -1;
        dir[i] = 0;
      }
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
                            const Rcpp::IntegerVector& divisions)
{
  PMap map(new Lattice(lower, upper, divisions));
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
