#include "../inst/include/Network.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"
#include <algorithm>

using namespace Rcpp;

Network::Network()
  : Contact(), Storage<std::vector<Agent *> >("Network")
{
}

const std::vector<Agent*> &Network::contact(double time, Agent &agent)
{
  auto *info = storage(agent);
  if (!info) throw std::runtime_error("agent not in network");
  return *info;
}

void Network::add(Agent &agent)
{
  auto *neighbors = storage(agent);
  if (!neighbors) store(agent, new std::vector<Agent*>());
  // have we finalized?
  if (_population != nullptr) grow(agent);
}

void Network::remove(Agent &agent)
{
  auto *neighbors = storage(agent);
  if (!neighbors) return;
  for (auto c : *neighbors) {
    auto *nj = storage(*c);
    assert(nj);
    size_t m = nj->size();
    for (size_t k = 0; k < m - 1; ++k) {
      if ((*nj)[k] == &agent) {
        nj[k] = nj[m-1];
        break;
      }
    }
    nj->resize(m-1);
  }
  neighbors->clear();
}

ConfigurationModel::ConfigurationModel(Function degree_rng)
  : Network(), _rng(degree_rng)
{
}

void ConfigurationModel::build()
{
  IntegerVector d = _rng(_population->size());
  size_t L = sum(d) + 0.5;
  std::vector<Agent *> stubs(L);
  for (size_t i = 0, k = 0; i < d.size(); ++i)
    for (size_t j = 0; j < d[i]; ++j)
      stubs[k++] = _population->agentAtIndex(i).get();
  size_t from, to, n = stubs.size();
  while (n > 2) {
    from = RUnif::stdUnif.get() * n;
    to = RUnif::stdUnif.get() * n;
    connect(stubs[from], stubs[to]);
    stubs[from] = stubs[n-1];
    stubs[to] = stubs[n-2];
    n -= 2;
  }
}

void Network::connect(Agent *from, Agent *to)
{
  if (&from == &to) return;
  // avoid multiple loops
  auto *fn = storage(*from);
  auto *tn = storage(*to);
  assert(fn != NULL && tn != NULL);
  for (auto c : *fn)
    if (c == to) return;
  fn->push_back(to);
  tn->push_back(from);
}

void ConfigurationModel::grow(Agent &agent)
{
  int degree = as<int>(_rng(1));
  std::vector<size_t> neighborhood(degree);
  size_t L = 0;
  for (size_t i = 0; i < _population->size(); ++i) {
    auto a = _population->agentAtIndex(i);
    auto *n = storage(*a);
    assert(n);
    L += n->size();
  }
  for (int j = 0; j < degree; ++j)
    neighborhood[j] = L * RUnif::stdUnif.get();
  std::sort(neighborhood.begin(), neighborhood.end());
  size_t k = 0, total = 0;
  for (size_t i = 0; i < _population->size(); ++i) {
    auto a = _population->agentAtIndex(i);
    auto *n = storage(*a);
    total += n->size();
    if (neighborhood[k] < total) {
      connect(&agent, a.get());
      if (k == degree) return;
    }
  }
}

// [[Rcpp::export]]
XP<ConfigurationModel> newConfigurationModel(Function rng)
{
  return XP<ConfigurationModel>(std::make_shared<ConfigurationModel>(rng));
}

