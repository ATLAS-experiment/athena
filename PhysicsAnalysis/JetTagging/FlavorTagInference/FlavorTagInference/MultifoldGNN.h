/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MULTIFOLD_GNN_H
#define MULTIFOLD_GNN_H

#include "FlavorTagInference/GNN.h"
#include "FlavorTagInference/GNNOptions.h"
#include "xAODJet/JetContainerFwd.h"

#include <vector>
#include <string>
#include <memory>
#include <set>

namespace FlavorTagInference {
  class GNN;
  struct GNNOptions;
}

namespace FlavorTagInference {

  class MultifoldGNN
  {
  public:
    MultifoldGNN(const std::vector<std::string>& folds,
                 const std::string& fold_hash_name,
                 const FlavorTagInference::GNNOptions& opts);
    MultifoldGNN(const std::vector<std::shared_ptr<const FlavorTagInference::GNN>>& folds,
                 const std::string& fold_hash_name);
    MultifoldGNN(MultifoldGNN&&);
    MultifoldGNN(const MultifoldGNN&);
    ~MultifoldGNN();
    void decorate(const xAOD::IParticle& i_jet) const;
    void decorateWithDefaults(const xAOD::IParticle& i_jet) const;

    // cppcheck-suppress returnByReference
    FTagDataDependencyNames getDependencies() const;
  private:
    const FlavorTagInference::GNN& getFold(const SG::AuxElement& element) const;
    std::vector<std::shared_ptr<const FlavorTagInference::GNN>> m_folds;
    SG::AuxElement::ConstAccessor<uint32_t> m_fold_hash;
    SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> m_jetLink;
  };

}
#endif
