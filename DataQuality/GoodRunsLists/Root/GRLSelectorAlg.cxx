/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// GoodRunsLists includes
#include <AsgMessaging/MessageCheck.h>
#include <EventBookkeeperTools/FilterReporter.h>
#include <GoodRunsLists/GRLSelectorAlg.h>
#include <xAODEventInfo/EventInfo.h>
#include "AsgDataHandles/WriteDecorHandle.h"

GRLSelectorAlg::GRLSelectorAlg( const std::string& name, ISvcLocator* pSvcLocator ) : AnaAlgorithm( name, pSvcLocator )
										    , m_grlTool("GoodRunsListSelectionTool", this)
{

  declareProperty( "Tool", m_grlTool, "The GoodRunsListSelectionTool" );

}


GRLSelectorAlg::~GRLSelectorAlg() {}


StatusCode GRLSelectorAlg::initialize() {
  //ATH_MSG_INFO ("Initializing " << name() << "...");
  ANA_CHECK( m_grlTool.retrieve() );
  ANA_CHECK( m_filterParams.initialize() );
  ANA_CHECK( m_grlKey.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode GRLSelectorAlg::finalize() {
  //ATH_MSG_INFO ("Finalizing " << name() << "...");
  ANA_MSG_INFO (m_filterParams.summary());
  return StatusCode::SUCCESS;
}

StatusCode GRLSelectorAlg::execute() {  
  FilterReporter filter (m_filterParams, m_noFilter.value());

  const xAOD::EventInfo* evtInfo = 0;
  ANA_CHECK( evtStore()->retrieve( evtInfo, "EventInfo" ) );
   
  SG::WriteDecorHandle<xAOD::EventInfo,char> dec_isGRLDecorator(m_grlKey);

  const bool isSelected = m_grlTool->passRunLB(*evtInfo);
  dec_isGRLDecorator(*evtInfo) = isSelected;
  if(!isSelected) return StatusCode::SUCCESS;
  
  filter.setPassed (true);

  return StatusCode::SUCCESS;
}
