/*
+  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MULTIFOLD_GNN_H
#define MULTIFOLD_GNN_H

#include "FlavorTagDiscriminants/GNNOptions.h"
#include "xAODBTagging/BTaggingFwd.h"
#include "xAODJet/JetContainerFwd.h"

#include <vector>
#include <string>
#include <memory>
#include <set>

namespace FlavorTagDiscriminants {

  class GNN;

  class MultifoldGNN
  {
  public:
    MultifoldGNN(const std::vector<std::string>& folds,
                 const std::string& fold_hash_name,
                 const GNNOptions& opts);
    MultifoldGNN(MultifoldGNN&&);
    MultifoldGNN(const MultifoldGNN&);
    ~MultifoldGNN();
    void decorate(const xAOD::BTagging& btag) const;
    void decorate(const xAOD::Jet& jet) const;
    void decorateWithDefaults(const SG::AuxElement& jet) const;

    std::set<std::string> getDecoratorKeys() const;
    std::set<std::string> getAuxInputKeys() const;
    std::set<std::string> getConstituentAuxInputKeys() const;
  private:
    const GNN& getFold(const SG::AuxElement& element) const;
    std::vector<std::shared_ptr<GNN>> m_folds;
    SG::AuxElement::ConstAccessor<uint32_t> m_fold_hash;
    SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> m_jetLink;
  };

}
#endif
