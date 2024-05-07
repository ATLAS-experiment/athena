/*
+  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/MultifoldGNN.h"
#include "FlavorTagDiscriminants/GNN.h"

#include "xAODBTagging/BTagging.h"
#include "xAODJet/JetContainer.h"

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
}

namespace FlavorTagDiscriminants {

  MultifoldGNN::MultifoldGNN(
    const std::vector<std::string>& nn_files,
    const std::string& fold_hash_name,
    const GNNOptions& o):
    m_fold_hash(fold_hash_name),
    m_jetLink(jetLinkName)
  {
    for (const auto& nn_file: nn_files) {
      m_folds.emplace_back(std::make_unique<GNN>(nn_file, o));
    }
  }
  MultifoldGNN::MultifoldGNN(MultifoldGNN&&) = default;
  MultifoldGNN::MultifoldGNN(const MultifoldGNN&) = default;
  MultifoldGNN::~MultifoldGNN() = default;

  void MultifoldGNN::decorate(const xAOD::BTagging& btag) const {
    getFold(**m_jetLink(btag)).decorate(btag);
  }
  void MultifoldGNN::decorate(const xAOD::Jet& jet) const {
    getFold(jet).decorate(jet);
  }
  void MultifoldGNN::decorateWithDefaults(const SG::AuxElement& jet) const {
    // note that the default values should be identical betwen all folds
    m_folds.at(0)->decorateWithDefaults(jet);
  }

  // Dependencies
  std::set<std::string> MultifoldGNN::getDecoratorKeys() const {
    return merged([](const auto& f){ return f.getDecoratorKeys(); }, m_folds);
  }
  std::set<std::string> MultifoldGNN::getAuxInputKeys() const {
    return merged([](const auto& f){ return f.getAuxInputKeys(); }, m_folds);
  }
  std::set<std::string> MultifoldGNN::getConstituentAuxInputKeys() const {
    return merged([](const auto& f){ return f.getConstituentAuxInputKeys(); }, m_folds);
  }

  const GNN& MultifoldGNN::getFold(const SG::AuxElement& element) const {
    return *m_folds.at(m_fold_hash(element) % m_folds.size());
  }


}
