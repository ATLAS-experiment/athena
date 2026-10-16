/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetPVNNSelection/PVNNTool.h"
#include "InDetPVNNSelection/PVNN.h"

namespace InDetPVNNSelection {

  PVNNTool::PVNNTool(const std::string& name):
          asg::AsgTool(name) {}

  PVNNTool::~PVNNTool() = default;

  StatusCode PVNNTool::initialize() {

    ATH_MSG_INFO("Initialize HS Tool (PVNN) from: " + m_nn_file);

    m_pvnn = std::make_unique<const PVNN>(m_nn_file);

    return StatusCode::SUCCESS;
  }

  float PVNNTool::decorate(const xAOD::Vertex& vertex) const {
    return m_pvnn->decorate(vertex);
  }

}
