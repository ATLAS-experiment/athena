/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/GNNTool.h"
#include "FlavorTagDiscriminants/GNN.h"
#include "FlavorTagDiscriminants/GNNOptions.h"

namespace FlavorTagDiscriminants {

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

    ATH_MSG_INFO("Initialize bTagging Tool (GNN) from: " + m_nn_file);

    m_gnn.reset(new GNN(m_nn_file, getOptions(m_props)));

    return StatusCode::SUCCESS;
  }

  void GNNTool::decorate(const xAOD::BTagging& btag) const {
    m_gnn->decorate(btag);
  }
  void GNNTool::decorate(const xAOD::Jet& jet) const {
    m_gnn->decorate(jet, jet);
  }
  void GNNTool::decorateWithDefaults(const SG::AuxElement& jet) const {
    m_gnn->decorateWithDefaults(jet);
  }

  void GNNTool::decorate(const xAOD::Jet& jet, const SG::AuxElement& btag) const
  {
    m_gnn->decorate(jet, btag);
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
