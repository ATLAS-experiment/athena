/*
+  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/MultifoldGNNTool.h"
#include "FlavorTagDiscriminants/MultifoldGNN.h"
#include "FlavorTagDiscriminants/GNNOptions.h"

namespace FlavorTagDiscriminants {

  MultifoldGNNTool::MultifoldGNNTool(const std::string& name):
          asg::AsgTool(name),
          m_props()
  {
    declareProperty("nnFiles", m_nn_files,
      "the path to the netowrk file used to run inference");
    declareProperty("foldHashName", m_fold_hash_name,
      "the path to the netowrk file used to run inference");
    propify(*this, &m_props);
  }

  MultifoldGNNTool::~MultifoldGNNTool() {}

  StatusCode MultifoldGNNTool::initialize() {

    ATH_MSG_INFO("Initialize multi-fold GNN");

    m_gnn.reset(
      new MultifoldGNN(
        m_nn_files,
        m_fold_hash_name,
        getOptions(m_props)
        )
      );

    return StatusCode::SUCCESS;
  }

  void MultifoldGNNTool::decorate(const xAOD::BTagging& btag) const {
    m_gnn->decorate(btag);
  }
  void MultifoldGNNTool::decorate(const xAOD::Jet& jet) const {
    m_gnn->decorate(jet);
  }
  void MultifoldGNNTool::decorateWithDefaults(const SG::AuxElement& jet) const {
    m_gnn->decorateWithDefaults(jet);
  }

  // Dependencies
  std::set<std::string> MultifoldGNNTool::getDecoratorKeys() const {
    return m_gnn->getDecoratorKeys();
  }
  std::set<std::string> MultifoldGNNTool::getAuxInputKeys() const {
    return m_gnn->getAuxInputKeys();
  }
  std::set<std::string> MultifoldGNNTool::getConstituentAuxInputKeys() const {
    return m_gnn->getConstituentAuxInputKeys();
  }

}
