#include "../inst/include/RNG.h"

using namespace Rcpp;

RealRN::RealRN(size_t cache_size)
  : _cache_size(cache_size == 0 ? 10000 : cache_size), _pos(cache_size)
{
}
  
double RealRN::get()
{
  if (_pos >= _cache_size) {
    RNGScope rngScope;
    _cache = refill(_cache_size);
    _pos = 0;
  }
  return _cache[_pos++];
}

RUnif::RUnif(double from, double to, size_t cache_size)
  : RealRN(cache_size), _from(from), _to(to)
{
}

NumericVector RUnif::refill(size_t size) {
  return runif(size, _from, _to);
}

RExp::RExp(double rate, size_t cache_size)
  : RealRN(cache_size), _rate(rate)
{
}

NumericVector RExp::refill(size_t size){
  return _rate == 0 ? NumericVector(size, R_PosInf) : rexp(size, _rate);
}
