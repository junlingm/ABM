#include "../inst/include/Network.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"
#include "../inst/include/Transition.h"
#include <algorithm>
#include <utility>

using namespace Rcpp;

Network::Network(std::string type, PWaitingTime waiting_time)
  : Contact(std::move(type), std::move(waiting_time))
{
}

const std::vector<Agent*> &Network::contact(double time, Agent &agent)
{
  return _neighbors[agent.index()];
}

void Network::add(Agent &agent)
{
  // have we finalized?
  if (_population != nullptr) grow(agent);
}

void Network::remove(Agent &agent)
{
  if (_population == nullptr) return;
  size_t i = agent.index();
  if (i >= _neighbors.size())
    stop("agent index is outside the network");
  for (auto c : _neighbors[i]) {
    size_t j = c->index();
    if (j >= _neighbors.size())
      stop("neighbor index is outside the network");
    std::vector<Agent*> &nj = _neighbors[j];
    auto pos = std::find(nj.begin(), nj.end(), &agent);
    if (pos != nj.end()) {
      *pos = nj.back();
      nj.pop_back();
    }
  }
  size_t last = _neighbors.size() - 1;
  if (i != last)
    _neighbors[i].swap(_neighbors[last]);
  _neighbors.pop_back();
}

void Network::build()
{
  size_t n = _population->size();
  _neighbors.resize(n);
  buildNetwork();
}

ConfigurationModel::ConfigurationModel(Function degree_rng, std::string type,
                                       PWaitingTime waiting_time)
  : Network(std::move(type), std::move(waiting_time)), _rng(degree_rng)
{
}

void ConfigurationModel::buildNetwork()
{
  IntegerVector d = _rng(_neighbors.size());
  if (d.size() != static_cast<R_xlen_t>(_neighbors.size()))
    stop("degree generator returned the wrong number of degrees");
  size_t L = 0;
  for (auto degree : d) {
    if (IntegerVector::is_na(degree) || degree < 0)
      stop("degrees must be non-negative integers");
    L += degree;
  }
  std::vector<int> stubs(L);
  for (size_t i = 0, k = 0; i < d.size(); ++i)
    for (size_t j = 0; j < d[i]; ++j)
      stubs[k++] = i;
  for (size_t n = stubs.size(); n > 1; --n) {
    size_t selected = _unif.get() * n;
    if (selected >= n) selected = n - 1;
    std::swap(stubs[n - 1], stubs[selected]);
  }
  // connect() drops self-loops and duplicate edges. If the number of stubs is
  // odd, the final unpaired stub is intentionally ignored.
  for (size_t i = 0; i + 1 < stubs.size(); i += 2)
    connect(stubs[i], stubs[i + 1]);
}

void Network::connect(int from, int to)
{
  if (from == to) return;
  // avoid multiple edges
  auto t = _population->agentAtIndex(to);
  for (auto c : _neighbors[from])
    if (c == t) return;
  _neighbors[from].push_back(t);
  _neighbors[to].push_back(_population->agentAtIndex(from));
}

void ConfigurationModel::grow(Agent &agent)
{
  Agent::IndexType i = agent.index();
  if (_neighbors.size() <= i)
    _neighbors.resize(i + 1);
  int degree = as<int>(_rng(1));
  if (degree <= 0) return;
  std::vector<size_t> neighborhood(degree);
  size_t L = 0;
  for (auto c : _neighbors)
    L += c.size();
  for (int j = 0; j < degree; ++j)
    neighborhood[j] = L * _unif.get();
  std::sort(neighborhood.begin(), neighborhood.end());
  size_t k = 0, total = 0;
  for (size_t j = 0; j < _neighbors.size() && k < neighborhood.size(); ++j) {
    total += _neighbors[j].size();
    while (k < neighborhood.size() && neighborhood[k] < total) {
      connect(i, j);
      ++k;
    }
  }
}

// [[Rcpp::export]]
XP<ConfigurationModel> newConfigurationModel(
    Function rng, SEXP rate = R_NilValue, std::string type = "contact")
{
  PWaitingTime waiting_time;
  if (rate != R_NilValue) {
    if (TYPEOF(rate) == EXTPTRSXP)
      waiting_time = as<XP<WaitingTime> >(rate);
    else if (Rf_isFunction(rate))
      waiting_time = makeOwned<RWaitingTime>(as<Function>(rate));
    else if (Rf_isNumeric(rate))
      waiting_time = makeOwned<ExpWaitingTime>(as<double>(rate));
    else
      stop("contact rate must be a waiting-time object, function, number, or NULL");
  }
  return XP<ConfigurationModel>(
    makeOwned<ConfigurationModel>(
      rng, std::move(type), std::move(waiting_time)));
}
