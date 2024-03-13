#pragma once

#include <Rcpp.h>

/**
 * A Abstract class for random number generators
 * 
 * The main use is to cache multiple random number generation in
 * one go, so that the random number states just needs to be initialized
 * once for each cache full of random numbers. When the cache is depleted
 * it is automaticallty refilled.
 */
class RealRN {
public:
  /**
   * Constructor
   * 
   * @param cache_size the cache size as a positive integer. 
   * 
   * @details The random numbers will be generated in batches of cache 
   * size, and will be returned one by one ultil the cache is depleted.
   */
  RealRN(size_t cache_size = 10000);

  /**
   * return a single random number from the cache.
   */  
  double get();  

protected:
  /**
   * a method to refill the cache.
   * 
   * @param size the number of random numbers to be generated in 
   * a batch
   * 
   * @return a NumericVector that holds the batch of generated random 
   * numbers 
   */
  virtual Rcpp::NumericVector refill(size_t size) = 0;
private:
  /**
   * the cache size
   */
  size_t _cache_size;
  /**
   * the current position in the cache
   */
  size_t _pos;
  /**
   * the cache for random numbers generated in a batch.
   */
  Rcpp::NumericVector _cache;
};

/**
 * A cached uniform random number generated
 */
class RUnif : public RealRN {
public:
  /**
   * Constructor
   * 
   * @param from the smallest random number to be generated
   * 
   * @param to the upper bound of the random numbers (all smaller than to)
   * 
   * @param cache_size the case size to generate random numebr in 
   * a batch
   */
  RUnif(double from = 0, double to = 1, size_t cache_size = 10000);

protected:
  /**
   * a method to refill the cache.
   * 
   * @param size the number of random numbers to be generated in 
   * a batch
   * 
   * @return a NumericVector that holds the batch of generated random 
   * numbers 
   */
  virtual Rcpp::NumericVector refill(size_t size);
  
private:
  double _from;
  double _to;
};

/**
 * A cached exponential random number generated
 */
class RExp : public RealRN {
public:
  /**
   * Constructor
   * 
   * @param rate the rate of the exponential random numbers
   * 
   * @param cache_size the case size to generate random numebr in 
   * a batch
   */
  RExp(double rate, size_t cache_size = 10000);
  
protected:
  /**
   * a method to refill the cache.
   * 
   * @param size the number of random numbers to be generated in 
   * a batch
   * 
   * @return a NumericVector that holds the batch of generated random 
   * numbers 
   */
  virtual Rcpp::NumericVector refill(size_t size);
  
private:
  /**
   * the scale (i.e., 1/rate) of the random numbers
   */
  double _rate;
};
