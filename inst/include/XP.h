#pragma once

#include <Rcpp.h>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>

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
 * A lifetime token for borrowed references passed to R. The scope that owns
 * the referenced object or callback owns the shared token; borrowed external
 * pointers retain only a weak reference.
 */
class XPLease {
};

typedef std::shared_ptr<XPLease> PXPLease;

/**
 * Abstract storage for external pointers in one polymorphic family.
 */
template<class T>
class Pointer {
public:
  virtual ~Pointer() = default;

  virtual T *checked() = 0;
  virtual const T *checked() const = 0;

  /** Returns an owning pointer, or nullptr for a borrowed handle. */
  virtual std::shared_ptr<T> shared() const = 0;
};

/**
 * External-pointer storage that shares ownership of its object.
 */
template<class T>
class SharedPointer final : public Pointer<T> {
public:
  explicit SharedPointer(std::shared_ptr<T> pointer)
    : _pointer(std::move(pointer))
  {
    if (!_pointer)
      Rcpp::stop("cannot create an ABM handle for a null object");
  }

  T *checked() override { return _pointer.get(); }
  const T *checked() const override { return _pointer.get(); }

  std::shared_ptr<T> shared() const override { return _pointer; }

private:
  std::shared_ptr<T> _pointer;
};

/**
 * External-pointer storage that borrows an object for a leased lifetime.
 */
template<class T>
class BorrowedPointer final : public Pointer<T> {
public:
  BorrowedPointer(T &pointer, const PXPLease &lease)
    : _pointer(&pointer), _lease(lease)
  {
    if (!lease)
      Rcpp::stop("cannot create a borrowed ABM handle without a lease");
  }

  T *checked() override { return checkedPointer(); }
  const T *checked() const override { return checkedPointer(); }

  std::shared_ptr<T> shared() const override { return nullptr; }

private:
  T *checkedPointer() const
  {
    if (_lease.expired())
      Rcpp::stop("ABM borrowed handle has expired");
    return _pointer;
  }

  T *_pointer;
  std::weak_ptr<XPLease> _lease;
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
        new SharedPointer<PointerBase>(
          std::static_pointer_cast<PointerBase>(p)),
        true,
        makeTag())
  {
    static_assert(std::is_base_of<PointerBase, T>::value,
                  "T must derive from T::PointerBase");
    this->attr("class") = p->classes;
  }

  XP(T &p, const PXPLease &lease)
    : XPtrBase(
        new BorrowedPointer<PointerBase>(
          static_cast<PointerBase&>(p), lease),
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
    std::shared_ptr<PointerBase> base = holder()->shared();
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
