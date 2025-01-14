#pragma once

#include <Rcpp.h>
#include <memory>

/**
 * A template class that encapsulates the external pointers
 * 
 * The memory management in this framework is done on C++ side for 
 * performance reasons. However, some objects created in C++ are returned
 * to R side for management, for example, newAgent returns an agent to
 * the R side. As such objects are not maintained on the C++ side, and
 * R is responsible for managing their lifespan, this is done with an 
 * external pointer holding a shared_ptr object to be passed to R. This 
 * allows the shared_ptr to manage the life span on the C++ side
 * 
 * However, some components of the framework, such as event handlers or 
 * transition callbacks can be R functions that needs to take the 
 * reference to the C++ objects. These references should not be managed on
 * the R side, and thus needs to be distinguished with the shared_ptr
 * above. On the other hand, both point to the same object, and thus
 * devising two interfaces for the same access can be confusing and error 
 * prune.
 * 
 * This template class solves this problem by holding both a shared_ptr 
 * and a normal pointer pointing to the C++ object. If the aim is to pass
 * reference, then the shared_ptr is nullptr. And this class does the
 * required translation to the required pointer types.
 */
template<class T>
class Pointer {
public:
  /**
   * Constructor for a shared_ptr pointing to a C++ object
   */
  Pointer(std::shared_ptr<T> p) : _p(p), __p(p.get()) { }
  /**
   * Constructor for a refernce to a C++ object.
   */
  Pointer(T &p) : __p(&p) { }

  /**
   * returns a const C++ pointer to the object.
   */
  operator const T*() const { return __p; }
  
  /**
   * returns C++ pointer to the object.
   */
  operator T*() { return __p; }
  
  /**
   * returns a shared_ptr to the object, or nullptr if the object
   * is passed by reference.
   */
  operator std::shared_ptr<T>() const { return _p; }

  /**
   * access to the C++ object
   */
  T *operator->() { return __p; }
  
  /**
   * const access to the C++ object
   */
  const T *operator->() const { return __p; }

  /**
   * returns whether this object points to a managed C++ object (true)
   * or a reference to asn object (false) 
   */
  bool managed() const { return bool(_p); }

private:
  /**
   * The shared_ptr pointing to a C++ object. Is nullptr if the object
   * passes a reference.
   */
  std::shared_ptr<T> _p;
  /**
   * holds to a pointer to a C++ object passing either a shared_ptr or
   * a reference.
   */
  T *__p;
};

/**
 * An external pointer that holds a Pointer object pointing to a C++
 * object either using a shared_ptr or passing by reference.
 */
template<class T>
class XP : public Rcpp::XPtr<Pointer<T> > {
public:
  /**
   * Constructor that converts from an R value
   * 
   * @param p an R object holding an external pointer to a Pointer object
   */
  XP(SEXP p)
    : Rcpp::XPtr<Pointer<T> >(p)
  {
  }

  /**
   * Constructor converts a shared_ptr to an external pointer
   * 
   * @param p a shared_ptr pointing a C++ object
   */
  XP(const std::shared_ptr<T> &p)
    : Rcpp::XPtr<Pointer<T> >(new Pointer<T>(p))
  {
    this->attr("class") = p->classes;
  }
  
  /**
   * Constructor that converts a C++ reference to an external pointer
   * 
   * @param p a C++ reference
   */
  XP(T &p)
    : Rcpp::XPtr<Pointer<T> >(new Pointer<T>(p))
  {
    this->attr("class") = p.classes;
  }

  /**
   * converts to a normal const C++ pointer
   */
  operator const T*() const { return *this->get(); }
  
  /**
   * converts to a normal C++ pointer
   */
  operator T*() { return *this->get(); }
  
  /**
   * converts to a shared_ptr
   * 
   * @details returns nullptr if the esternal pointer holding a reference
   * to a C++ object.
   */
  operator std::shared_ptr<T> () const { return *this->get(); }
  
  /**
   * access to the C++ object
   */
  T *operator->() { return *this->get(); }

  /**
   * const access to the C++ object
   */
  const T *operator->() const { return *this->get(); }

};
