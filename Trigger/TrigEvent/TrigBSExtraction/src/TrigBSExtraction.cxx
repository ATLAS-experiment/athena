/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "TrigBSExtraction/TrigBSExtraction.h"
#include "TrigSteeringEvent/HLTResult.h"
#include "TrigSerializeCnvSvc/TrigSerializeConvHelper.h"

TrigBSExtraction::TrigBSExtraction(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
{
}


StatusCode TrigBSExtraction::initialize() {

  ATH_CHECK( m_navTool.retrieve() );

  // Initialize handle keys
  ATH_CHECK( m_hltResultKeyIn.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_hltResultKeyOut.initialize(SG::AllowEmpty) );

  if ( m_dataScoutingKeysIn.size() != m_dataScoutingKeysOut.size() ) {
    ATH_MSG_ERROR("Size of DSResultKeysIn/Out does not match: " << m_dataScoutingKeysIn <<
                  " vs " << m_dataScoutingKeysOut );
    return StatusCode::FAILURE;
  }
  ATH_CHECK( m_dataScoutingKeysIn.initialize() );
  ATH_CHECK( m_dataScoutingKeysOut.initialize() );

  // Initialize the TrigSerializeConcHelper here so that all the additional streamerinfos are read in.
  // In case of DataScouting this may give problems if the DS ROBs contain containers which
  // were not compiled with navigation (the container type information is then not known
  // for a given CLID).
  ToolHandle<ITrigSerializeConvHelper> trigCnvHelper("TrigSerializeConvHelper",this);
  ATH_CHECK( trigCnvHelper.retrieve() );

  return StatusCode::SUCCESS;
}


StatusCode TrigBSExtraction::execute(const EventContext& ctx) {

  if ( !m_hltResultKeyIn.empty() ) {
    if ( repackFeaturesToSG(ctx, *m_navTool, m_hltResultKeyIn, m_hltResultKeyOut).isFailure() )
      ATH_MSG_WARNING( "failed unpacking features from BS to SG for: " << m_hltResultKeyIn  );
  }

  for (size_t i = 0; i<m_dataScoutingKeysIn.size(); i++ ) {
    if ( repackFeaturesToSG(ctx, *m_navTool, m_dataScoutingKeysIn[i], m_dataScoutingKeysOut[i]).isFailure() )
      ATH_MSG_WARNING( "failed unpacking features from BS to SG for: " << m_dataScoutingKeysIn[i] );
  }
  m_navTool->reset();

  return StatusCode::SUCCESS;
}


StatusCode TrigBSExtraction::repackFeaturesToSG (const EventContext& ctx,
                                                 HLT::Navigation& navTool,
                                                 const SG::ReadHandleKey<HLT::HLTResult>& key,
                                                 SG::WriteHandleKey<HLT::HLTResult>& keyOut) {

  ATH_MSG_DEBUG( "Trying to deserialize content of " << key );
  auto cresult = SG::makeHandle(key, ctx);

  if ( !cresult.get() ) {
    ATH_MSG_WARNING( "No HLTResult found with key " << key );
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("HLTResult is level=" << cresult->getHLTLevel()  );

  const std::vector<uint32_t>& navData = cresult->getNavigationResult();
  if ( !navData.empty() ) {
    ATH_MSG_DEBUG( "Navigation payload obtained from " << key << " has size " << navData.size()  );
    navTool.deserialize( navData );
  } else {
    ATH_MSG_WARNING( "Navigation payload obtained from " << key << " has size 0" );
  }

  navTool.prepare();

  // Create a copy of HLTResult and pack navigation back into it
  auto result = std::make_unique<HLT::HLTResult>(*cresult);

  result->getNavigationResult().clear();
  bool status = navTool.serialize(result->getNavigationResult(), result->getNavigationResultCuts());
  ATH_MSG_DEBUG( "New serialized navigation for " << keyOut << " has size " << result->getNavigationResult().size() );

  ATH_CHECK( SG::makeHandle(keyOut, ctx).record(std::move(result)) );

  return StatusCode(status);
}

