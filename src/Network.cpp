#include "../inst/include/Network.h"
#include "../inst/include/Population.h"
#include "../inst/include/RNG.h"

using namespace Rcpp;

Network::Network()
  : Contact()
{
}

const std::vector<PAgent> &Network::contact(double time, Agent &agent)
{
  return _neighbors[agent.id() - 1];
}

void Network::add(const PAgent &agent)
{
  // have we finalized?
  if (_population != nullptr) grow(agent);
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
    from = RUnif::stdUnif.get() * n;
    to = RUnif::stdUnif.get() * n;
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
  auto t = _population->agent(to);
  for (auto c : _neighbors[from])
    if (c == t) return;
  _neighbors[from].push_back(t);
  _neighbors[to].push_back(_population->agent(from));
}

void ConfigurationModel::grow(const PAgent &agent)
{
  stop("not implemented yet");
}

// [[Rcpp::export]]
XP<ConfigurationModel> newConfigurationModel(Function rng)
{
  return XP<ConfigurationModel>(std::make_shared<ConfigurationModel>(rng));
}

