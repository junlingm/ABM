#include "../inst/include/Network.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"
#include <algorithm>

using namespace Rcpp;

Network::Network()
  : Contact()
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
  size_t i = agent.id() - 1;
  for (auto c : _neighbors[i]) {
    size_t j = c->id() - 1;
    std::vector<Agent*> &nj = _neighbors[j];
    size_t m = nj.size();
    for (size_t k = 0; k < m - 1; ++k) {
      if (nj[k] == &agent) {
        nj[k] = nj[m-1];
        break;
      }
    }
    nj.resize(m-1);
  }
  _neighbors[i].clear();
}

void Network::build()
{
  size_t n = _population->size();
  _neighbors.resize(n);
  buildNetwork();
}

ConfigurationModel::ConfigurationModel(Function degree_rng)
  : Network(), _rng(degree_rng)
{
}

void ConfigurationModel::buildNetwork()
{
  IntegerVector d = _rng(_neighbors.size());
  size_t L = sum(d) + 0.5;
  std::vector<int> stubs(L);
  for (size_t i = 0, k = 0; i < d.size(); ++i)
    for (size_t j = 0; j < d[i]; ++j)
      stubs[k++] = i;
  size_t from, to, n = stubs.size();
  while (n > 2) {
    from = _unif.get() * n;
    to = _unif.get() * n;
    connect(stubs[from], stubs[to]);
    stubs[from] = stubs[n-1];
    stubs[to] = stubs[n-2];
    n -= 2;
  }
}

void Network::connect(int from, int to)
{
  if (from == to) return;
  // avoid multiple loops
  auto t = _population->agentAtIndex(to).get();
  for (auto c : _neighbors[from])
    if (c == t) return;
  _neighbors[from].push_back(t);
  _neighbors[to].push_back(_population->agentAtIndex(from).get());
}

void ConfigurationModel::grow(Agent &agent)
{
  Agent::IndexType i = agent.index();
  int degree = as<int>(_rng(1));
  std::vector<size_t> neighborhood(degree);
  size_t L = 0;
  for (auto c : _neighbors)
    L += c.size();
  for (int j = 0; j < degree; ++j)
    neighborhood[j] = L * _unif.get();
  std::sort(neighborhood.begin(), neighborhood.end());
  size_t k = 0, total = 0;
  for (auto c : _neighbors) {
    total += c.size();
    if (neighborhood[k] < total) {
      connect(i, k++);
      if (k == degree) return;
    }
  }
}

// [[Rcpp::export]]
XP<ConfigurationModel> newConfigurationModel(Function rng)
{
  return XP<ConfigurationModel>(std::make_shared<ConfigurationModel>(rng));
}

