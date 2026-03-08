/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "JetMonitoring/EventHistoVarTool.h"
#include "StoreGate/ReadDecorHandle.h"


EventHistoVarTool::EventHistoVarTool(const std::string & type, const std::string & name ,const IInterface* parent):
  AthAlgTool( type, name, parent )
{
  declareInterface<IEventHistoVarTool>(this);
}

StatusCode EventHistoVarTool::initialize() {

  if(m_varName=="") m_varName = name();

  size_t pos = (m_attName.key()).find('.');
  if (pos == std::string::npos){
      m_attName = "EventInfo."+m_attName.key(); // If no container specified, assuming we want "EventInfo"
      ATH_MSG_DEBUG("Updated attribute key to "<<m_attName);
  }
  ATH_MSG_DEBUG("Event info attribute that will be retrieved: "<<m_attName);
  ATH_CHECK( m_attName.initialize() );

  return StatusCode::SUCCESS;
  
}

float EventHistoVarTool::value(const xAOD::JetContainer & /*jets not used in this implementation*/) const {

  SG::ReadDecorHandle<xAOD::EventInfo,float> eiDecor(m_attName);
  if(!eiDecor.isPresent()){
    ATH_MSG_WARNING("Could not access EventInfo variable "<< m_attName << ". Returning default value " << m_defaultValue );
    return m_defaultValue;
  }
  return eiDecor(0);
}
