/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "JetMomentTools/BoostedJetTaggerTool.h"

BoostedJetTaggerTool::BoostedJetTaggerTool(const std::string& name)
  : asg::AsgTool(name){}

StatusCode BoostedJetTaggerTool::initialize()
{
  ATH_MSG_INFO("Initialising BoostedJetTaggerTool " << name());
  
  ATH_MSG_INFO("BoostedJetTaggerTool: decorator name: " << m_decorationName);
  
  if(!m_MLTagger.empty()){
    asg::AsgToolConfig config ("JSSTaggerUtils/MLHelper");
    
    ATH_CHECK(config.setProperty("ContainerName", m_jetContainerName.value()));
    ATH_CHECK(config.setProperty("Decoration", m_decorationName.value()));
    ATH_CHECK(config.setProperty("CalibArea", m_calibArea.value()));
    ATH_CHECK(config.setProperty("ConfigFile", m_configFile.value()));
    
    ATH_CHECK(config.makePrivateTool(m_MLTagger) );
    ATH_CHECK(m_MLTagger.retrieve());
  }
  
  return StatusCode::SUCCESS;
}

StatusCode BoostedJetTaggerTool::decorate(const xAOD::JetContainer& jets) const
{
  
  // decorate all the jets with the score
  if ( m_decorationName.value().compare("QGTransformer") == 0 )
    ATH_CHECK(m_MLTagger -> GetQGConstScore(jets));

  return StatusCode::SUCCESS;
}
