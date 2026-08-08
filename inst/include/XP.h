#pragma once

#include <Rcpp.h>
#include <cstdint>
#include <memory>
#include <type_traits>

/**
 * Capabilities supported by objects exposed through external pointers.
 * Derived classes include the capabilities of their base classes in TAG.
 */
enum XPTag : std::uint32_t {
  XP_EVENT        = 1u << 0,
  XP_CALENDAR     = 1u << 1,
  XP_AGENT        = 1u << 2,
  XP_POPULATION   = 1u << 3,
  XP_SIMULATION   = 1u << 4,
  XP_CONTACT      = 1u << 5,
  XP_LOGGER       = 1u << 6,
  XP_EVENT_LOGGER = 1u << 7,
  XP_WAITING_TIME = 1u << 8
};

/**
 * A lifetime token for references passed to an R callback. The callback owns
 * the shared token; borrowed external pointers retain only a weak reference.
 */
class XPLease {
};

typedef std::shared_ptr<XPLease> PXPLease;

/**
 * Storage shared by all external-pointer types in one polymorphic family.
 *
 * Managed pointers own the object through _p. Borrowed pointers keep a raw
 * pointer and may optionally be limited by a callback lease.
 */
template<class T>
class Pointer {
public:
  explicit Pointer(std::shared_ptr<T> p)
    : _p(std::move(p)), _borrowed(nullptr), _scoped(false)
  {
    if (!_p)
      Rcpp::stop("cannot create an ABM handle for a null object");
  }

  explicit Pointer(T &p)
    : _borrowed(&p), _scoped(false)
  {
  }

  Pointer(T &p, const PXPLease &lease)
    : _borrowed(&p), _lease(lease), _scoped(true)
  {
    if (!lease)
      Rcpp::stop("cannot create an ABM callback handle without a lease");
  }

  T *checked()
  {
    if (_p)
      return _p.get();
    if (_borrowed == nullptr)
      Rcpp::stop("ABM handle has no object");
    if (_scoped && _lease.expired())
      Rcpp::stop("ABM callback handle has expired");
    return _borrowed;
  }

  const T *checked() const
  {
    if (_p)
      return _p.get();
    if (_borrowed == nullptr)
      Rcpp::stop("ABM handle has no object");
    if (_scoped && _lease.expired())
      Rcpp::stop("ABM callback handle has expired");
    return _borrowed;
  }

  operator const T*() const { return checked(); }
  operator T*() { return checked(); }

  /** Returns nullptr for borrowed pointers, including scoped borrows. */
  operator std::shared_ptr<T>() const { return _p; }

  T *operator->() { return checked(); }
  const T *operator->() const { return checked(); }

  bool managed() const { return bool(_p); }

private:
  std::shared_ptr<T> _p;
  T *_borrowed;
  std::weak_ptr<XPLease> _lease;
  bool _scoped;
};

/**
 * An external pointer to an ABM object.
 *
 * T must define:
 *   - PointerBase: the polymorphic root stored by its pointer family
 *   - TAG: the capability bits required to use the object as T
 */
template<class T>
class XP : public Rcpp::XPtr<Pointer<typename T::PointerBase> > {
private:
  typedef typename T::PointerBase PointerBase;
  typedef Pointer<PointerBase> Holder;
  typedef Rcpp::XPtr<Holder> XPtrBase;

  static Rcpp::IntegerVector makeTag()
  {
    return Rcpp::IntegerVector::create(static_cast<int>(T::TAG));
  }

  static void validateTag(SEXP p)
  {
    SEXP tag = R_ExternalPtrTag(p);
    if (TYPEOF(tag) != INTSXP || Rf_xlength(tag) != 1 ||
        INTEGER(tag)[0] == NA_INTEGER)
      Rcpp::stop("invalid ABM external-pointer tag");

    std::uint32_t actual = static_cast<std::uint32_t>(INTEGER(tag)[0]);
    std::uint32_t required = static_cast<std::uint32_t>(T::TAG);
    if ((actual & required) != required)
      Rcpp::stop("ABM external pointer does not support the requested type");
  }

  Holder *holder()
  {
    return XPtrBase::checked_get();
  }

  const Holder *holder() const
  {
    return XPtrBase::checked_get();
  }

  T *checked()
  {
    PointerBase *base = holder()->checked();
    T *p = dynamic_cast<T*>(base);
    if (p == nullptr)
      Rcpp::stop("ABM external pointer has an incompatible C++ type");
    return p;
  }

  const T *checked() const
  {
    const PointerBase *base = holder()->checked();
    const T *p = dynamic_cast<const T*>(base);
    if (p == nullptr)
      Rcpp::stop("ABM external pointer has an incompatible C++ type");
    return p;
  }

public:
  explicit XP(SEXP p)
    : XPtrBase(p)
  {
    validateTag(p);
  }

  XP(const std::shared_ptr<T> &p)
    : XPtrBase(
        new Holder(std::static_pointer_cast<PointerBase>(p)),
        true,
        makeTag())
  {
    static_assert(std::is_base_of<PointerBase, T>::value,
                  "T must derive from T::PointerBase");
    this->attr("class") = p->classes;
  }

  XP(T &p)
    : XPtrBase(
        new Holder(static_cast<PointerBase&>(p)),
        true,
        makeTag())
  {
    static_assert(std::is_base_of<PointerBase, T>::value,
                  "T must derive from T::PointerBase");
    this->attr("class") = p.classes;
  }

  XP(T &p, const PXPLease &lease)
    : XPtrBase(
        new Holder(static_cast<PointerBase&>(p), lease),
        true,
        makeTag())
  {
    static_assert(std::is_base_of<PointerBase, T>::value,
                  "T must derive from T::PointerBase");
    this->attr("class") = p.classes;
  }

  operator const T*() const { return checked(); }
  operator T*() { return checked(); }

  operator std::shared_ptr<T>() const
  {
    std::shared_ptr<PointerBase> base = *holder();
    if (!base)
      return nullptr;
    std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(base);
    if (!p)
      Rcpp::stop("ABM external pointer has an incompatible C++ type");
    return p;
  }

  T &operator*() { return *checked(); }
  const T &operator*() const { return *checked(); }

  T *operator->() { return checked(); }
  const T *operator->() const { return checked(); }
};
