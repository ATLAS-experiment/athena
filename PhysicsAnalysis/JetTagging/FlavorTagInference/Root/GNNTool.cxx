/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/GNNTool.h"
#include "FlavorTagInference/GNN.h"
#include "FlavorTagInference/GNNOptions.h"

namespace FlavorTagInference {

  GNNTool::GNNTool(const std::string& name):
          asg::AsgTool(name),
          m_props()
  {
    declareProperty("nnFile", m_nn_file,
      "the path to the netowrk file used to run inference");
    propify(*this, &m_props);
  }

  GNNTool::~GNNTool() {}

  StatusCode GNNTool::initialize() {

    auto opts = getOptions(m_props);
    if (!m_nnsvc.empty()) {
      ATH_CHECK(m_nnsvc.retrieve());
      m_gnn = m_nnsvc->get(m_nn_file, opts);
    } else {
      ATH_MSG_INFO("Initialize bTagging Tool (GNN) from: " + m_nn_file);
      m_gnn.reset(new GNN(m_nn_file, opts));
    }

    return StatusCode::SUCCESS;
  }

  void GNNTool::decorate(const xAOD::IParticle& i_jet) const {
    m_gnn->decorate(i_jet);
  }
  void GNNTool::decorateWithDefaults(const xAOD::IParticle& i_jet) const {
    m_gnn->decorateWithDefaults(i_jet);
  }

  // Dependencies
  std::set<std::string> GNNTool::getDecoratorKeys() const {
    return m_gnn->getDecoratorKeys();
  }
  std::set<std::string> GNNTool::getAuxInputKeys() const {
    return m_gnn->getAuxInputKeys();
  }
  std::set<std::string> GNNTool::getConstituentAuxInputKeys() const {
    return m_gnn->getConstituentAuxInputKeys();
  }

}
