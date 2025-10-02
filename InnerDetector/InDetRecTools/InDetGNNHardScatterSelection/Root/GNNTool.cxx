/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/GNNTool.h"
#include "InDetGNNHardScatterSelection/GNN.h"

namespace InDetGNNHardScatterSelection {

  GNNTool::GNNTool(const std::string& name):
          asg::AsgTool(name) {}

  GNNTool::~GNNTool() = default;

  StatusCode GNNTool::initialize() {

    ATH_MSG_INFO("Initialize HS Tool (GNN) from: " + m_nn_file);

    m_gnn = std::make_unique<const GNN>(m_nn_file);

    return StatusCode::SUCCESS;
  }

  void GNNTool::decorate(const xAOD::Vertex& vertex) const {
    m_gnn->decorate(vertex);
  }

}
