/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/DL2Tool.h"
#include "FlavorTagDiscriminants/DL2HighLevel.h"

namespace FlavorTagDiscriminants {

  DL2Tool::DL2Tool(const std::string& name):
    asg::AsgTool(name),
    m_props(),
    m_dl2(nullptr)
  {
    declareProperty("nnFile", m_props.nnFile);
    declareProperty("flipTagConfig", m_props.flipTagConfig);
    declareProperty("variableRemapping", m_props.variableRemapping);
    declareProperty("defaultOutputValue", m_props.default_output_value);
  }
  DL2Tool::~DL2Tool() {}

  StatusCode DL2Tool::initialize() {
    ATH_MSG_INFO("Initialize DL2 from: " + m_props.nnFile);
    FlipTagConfig flipConfig = FlipTagConfig::STANDARD;
    if (m_props.flipTagConfig.size() > 0) {
      flipConfig = FlavorTagInference::flipTagConfigFromString(m_props.flipTagConfig);
    }
    m_dl2.reset(
      new DL2HighLevel(
        m_props.nnFile,
        flipConfig,
        m_props.variableRemapping
        )
      );
    return StatusCode::SUCCESS;
  }

  void DL2Tool::decorate(const xAOD::IParticle& i_jet) const {
    ATH_MSG_DEBUG("Decorating i_jet from: " + m_props.nnFile);
    m_dl2->decorate(i_jet);
    ATH_MSG_VERBOSE("Decorated i_jet");
  }
  void DL2Tool::decorateWithDefaults(const xAOD::IParticle& i_jet) const {
    ATH_MSG_DEBUG("Decorating i_jet with defaults from: " + m_props.nnFile);
    m_dl2->decorateWithDefaults(i_jet);
    ATH_MSG_VERBOSE("Decorated i_jet with defaults");
  }

  FTagDataDependencyNames DL2Tool::getDependencies() const {
    return m_dl2->getDataDependencyNames();
  }

}
