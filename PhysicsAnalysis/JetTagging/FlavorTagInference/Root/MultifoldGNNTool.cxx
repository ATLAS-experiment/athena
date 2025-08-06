/*
+  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/MultifoldGNNTool.h"
#include "FlavorTagInference/MultifoldGNN.h"
#include "FlavorTagInference/GNNOptions.h"

namespace FlavorTagInference {

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

    const auto opts = FlavorTagInference::getOptions(m_props);
    if (!m_nnsvc.empty()) {
      ATH_CHECK(m_nnsvc.retrieve());
      std::vector<std::shared_ptr<const FlavorTagInference::GNN>> gnns;
      for (const auto& file: m_nn_files) {
        auto newopts = opts;
        if (auto def_handle = m_defaults.value().extract(file)) {
          newopts.default_output_values = def_handle.mapped();
        }
        gnns.emplace_back(m_nnsvc->get(file, newopts));
      }
      if (!m_defaults.empty()) {
        ATH_MSG_ERROR("unused per-fold defaults!");
        return StatusCode::FAILURE;
      }
      m_gnn.reset(new MultifoldGNN(gnns, m_fold_hash_name));
    } else {
      ATH_MSG_INFO("Initialize multi-fold GNN");
      m_gnn.reset(new MultifoldGNN(m_nn_files, m_fold_hash_name, opts));
    }

    return StatusCode::SUCCESS;
  }

  void MultifoldGNNTool::decorate(const xAOD::IParticle& i_jet) const {
    m_gnn->decorate(i_jet);
  }
  void MultifoldGNNTool::decorateWithDefaults(const xAOD::IParticle& i_jet) const {
    m_gnn->decorateWithDefaults(i_jet);
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
