#include "../inst/include/State.h"
#include <string>

using namespace Rcpp;

State::State()
  : List()
{
  this->attr("class") = "State";
}

State::State(Rcpp::List values)
  : List(values)
{
  this->attr("class") = "State";
}
  
State::State(const std::string name, SEXP value)
  : State()
{
  (*this)[name] = value;
}

State::State(const std::string name, std::string value)
  : State(name, CharacterVector::create(value))
{
}

State::State(const std::string name, double value)
  : State(name, NumericVector::create(value))
{
}

static bool all(const LogicalVector &x) {
  for (auto v : x)
    if (!v) return false;
  return true;
}

template <int RTYPE>
LogicalVector compare(Vector<RTYPE> x, Vector<RTYPE> y) {
  return x == y;
}

bool comp(SEXP x, SEXP y) {
  bool ok;
  if (y == R_NilValue)
    return true;
  if (Rf_isFunction(y)) {
    Function f(y);
    return all(f(x));
  }
  auto t = TYPEOF(y);
  if (TYPEOF(x) == t) {
    switch (t) {
    case INTSXP:
      ok = all(compare<INTSXP>(x, y));
      break;
    case REALSXP:
      ok = all(compare<REALSXP>(x, y));
      break;
    case STRSXP:
      ok = all(compare<STRSXP>(x, y));
      break;
    default:
      ok = false;
    }
  } else return false;
  return ok;
}

bool State::match(const Rcpp::List &rule) const
{
  SEXP rule_ns = rule.names();
  if (rule_ns == R_NilValue) {
    SEXP my_ns = names();
    if (my_ns == R_NilValue) {
      size_t n = rule.size();
      if (n == 0) return true;
      if (size() == 0) return false;
      return comp((*this)[0], rule[0]);
    }
    CharacterVector ns(my_ns);
    for (size_t i = 0; i < ns.size(); ++i) {
      if (ns[i] == "") {
        return comp((*this)[i], rule[0]);
      }
    }
    return false;
  }
  CharacterVector ns(rule_ns);
  for (auto n : ns) {
    std::string name = as<std::string>(n);
    if (!containsElementNamed(name.c_str())) return false;
    auto x = (*this)[name];
    SEXP y = rule[name];
    if (!comp(x, y)) return false;
  }
  return true;
}

State State::operator&(const List &y) const
{
  State x(clone(*this));
  return x &= y;
}

State &State::operator&=(const List &y)
{
  if (y.size() > 0) {
    SEXP y_ns = y.names();
    if (y_ns == R_NilValue) {
      SEXP my_ns = names();
      if (my_ns == R_NilValue) {
        if (size() == 0)
          push_back(y[0]);
        else (*this)[0] = y[0];
      } else {
        CharacterVector ns(my_ns);
        for (size_t i = 0; i < ns.size(); ++i) {
          if (ns[i] == "") {
            (*this)[i] = y[0];
            break;
          }
        }
      }
    } else {
      CharacterVector ns = y.names();
      for (auto n : ns) {
        std::string name = as<std::string>(n);
        (*this)[name] = y[name];
      }
    }
  }
  return *this;
}

// [[Rcpp::export]]
bool stateMatch(List state, SEXP rule)
{
  if (Rf_isFunction(rule)) {
    Function f(rule);
    return f(state);
  }
  List r(rule);
  return State(state).match(r);
}
