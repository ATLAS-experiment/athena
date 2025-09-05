/*
+  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/MultifoldGNN.h"
#include "FlavorTagInference/GNN.h"

#include "xAODJet/JetContainer.h"

using namespace FlavorTagInference;

namespace {
  const std::string jetLinkName = "jetLink";
  template<typename T, typename C>
  std::set<std::string> merged(T get, const C& c) {
    auto first = get(*c.at(0));
    for (size_t idx = 1; idx < c.size(); idx++) {
      if (get(*c.at(idx)) != first) {
        throw std::runtime_error("inconsistent dependencies in folds");
      }
    }
    return first;
  }
  auto getNNs(
    const std::vector<std::string>& nn_files,
    const GNNOptions& o)
  {
    std::vector<std::shared_ptr<const GNN>> nns;
    for (const auto& nn_file: nn_files) {
      nns.emplace_back(std::make_shared<const GNN>(nn_file, o));
    }
    return nns;
  }
}

namespace FlavorTagInference {

  MultifoldGNN::MultifoldGNN(
    const std::vector<std::string>& nn_files,
    const std::string& fold_hash_name,
    const GNNOptions& o):
    MultifoldGNN(getNNs(nn_files, o), fold_hash_name)
  {
  }
  MultifoldGNN::MultifoldGNN(
    const std::vector<std::shared_ptr<const GNN>>& nns,
    const std::string& fold_hash_name):
    m_folds(nns),
    m_fold_hash(fold_hash_name),
    m_jetLink(jetLinkName)
  {
  }
  MultifoldGNN::MultifoldGNN(MultifoldGNN&&) = default;
  MultifoldGNN::MultifoldGNN(const MultifoldGNN&) = default;
  MultifoldGNN::~MultifoldGNN() = default;

  void MultifoldGNN::decorate(const xAOD::IParticle& i_jet) const {
    getFold(i_jet).decorate(i_jet);
  }
  void MultifoldGNN::decorateWithDefaults(const xAOD::IParticle& i_jet) const {
    getFold(i_jet).decorateWithDefaults(i_jet);
  }

  // Dependencies
  std::set<std::string> MultifoldGNN::getDecoratorKeys() const {
    return merged([](const auto& f){ return f.getDecoratorKeys(); }, m_folds);
  }
  std::set<std::string> MultifoldGNN::getAuxInputKeys() const {
    auto out = merged([](const auto& f){ return f.getAuxInputKeys(); }, m_folds);
    out.insert(SG::AuxTypeRegistry::instance().getName(m_fold_hash.auxid()));
    return out;
  }
  std::set<std::string> MultifoldGNN::getConstituentAuxInputKeys() const {
    return merged([](const auto& f){ return f.getConstituentAuxInputKeys(); }, m_folds);
  }

  const GNN& MultifoldGNN::getFold(const SG::AuxElement& element) const {
    return *m_folds.at(m_fold_hash(element) % m_folds.size());
  }


}
