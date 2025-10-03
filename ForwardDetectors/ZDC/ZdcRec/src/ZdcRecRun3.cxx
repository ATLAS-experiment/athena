/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecRun3.cxx
 *
 *  Created on: June 23, 2022 (never forget)
 *      Author: steinberg@bnl.gov
 */


#include <memory>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"

#include "ZdcRec/ZdcRecRun3.h"
#include "xAODForward/ZdcModuleAuxContainer.h"
#include "xAODForward/ZdcModuleToString.h"
#include "ZdcByteStream/ZdcToString.h"
#include "ZdcUtils/ZdcEventInfo.h"

//==================================================================================================
ZdcRecRun3::ZdcRecRun3(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{}

//==================================================================================================

//==================================================================================================
ZdcRecRun3::~ZdcRecRun3() {}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::initialize()
{
  MsgStream mLog(msgSvc(), name());
  
  // Reconstruction Tool chain
  
  ATH_CHECK( m_zdcTools.retrieve() );
  
  ATH_CHECK( m_zdcModuleContainerName.initialize() );
  ATH_CHECK( m_zdcSumContainerName.initialize() );
  
  if (m_ownPolicy == SG::OWN_ELEMENTS)
    mLog << MSG::DEBUG << "...will OWN its cells." << endmsg;
  else
    mLog << MSG::DEBUG << "...will VIEW its cells." << endmsg;
  
  
  mLog << MSG::DEBUG << "--> ZDC: ZdcRecRun3 initialization complete" << endmsg;
  
  if (m_DAQMode >= ZdcEventInfo::numDAQModes) {
    ATH_MSG_ERROR("Invalid DAQ mode, mode = " <<  m_DAQMode);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG ("Configuration:");
  ATH_MSG_DEBUG("ForcedEventType = " << m_forcedEventType);
  ATH_MSG_DEBUG("DAQMode = " << m_DAQMode);

  // initialize eventInfo access
  //
  ATH_CHECK( m_eventInfoKey.initialize());
  ATH_CHECK( m_eventInfoDecorKey.initialize() );
  
  // Initialize writedecor keys
  //
  std::string sumContainerName = "ZdcSums";
  m_ZdcEventType = sumContainerName + ".EventType";
  ATH_CHECK( m_ZdcEventType.initialize());

  m_ZdcDAQMode = sumContainerName + ".DAQMode";
  ATH_CHECK( m_ZdcDAQMode.initialize());
  
  return StatusCode::SUCCESS;
}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::execute()
{

  ATH_MSG_DEBUG("In ZdRecRun3");

  const EventContext& ctx = Gaudi::Hive::currentContext();

  ATH_MSG_DEBUG ("--> ZDC: ZdcRecRun3 execute starting on "
                 << ctx.evt()
                 << "th event");

  // Get event info
  //
  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfoKey);
  if (!eventInfo.isValid()) return StatusCode::FAILURE;

  if (eventInfo->errorState(xAOD::EventInfo::ForwardDet)==xAOD::EventInfo::Error)
    {
      if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::DECODINGERROR ))
	{
	  bool rpdErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR );
	  bool zdcErr = eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR );
	  std::string errs = "";
	  if (zdcErr) errs += "* ZDC ";
	  if (rpdErr) errs += "* RPD ";
	  ATH_MSG_WARNING("Decoding error in LUCROD decoding:"+errs);
	  // We no longer abandon the event now - the separate RPD and ZDC analysis tools check the errors and abandon the event separately.
	  // return StatusCode::SUCCESS;
	}
    }

  ATH_MSG_DEBUG("Event info type=IS_CALIBRATION:" << eventInfo->eventType(xAOD::EventInfo::IS_CALIBRATION));

  //
  // Figure out what kind of event this is
  //
  unsigned int eventType = ZdcEventInfo::ZdcEventUnknown;

  if (m_forcedEventType != ZdcEventInfo::ZdcEventUnknown) {
    eventType = m_forcedEventType;
  }
  else {
    if (m_DAQMode == ZdcEventInfo::Standalone) {
      //
      // Problem: we can't determine event type in standalone mode
      //
      ATH_MSG_FATAL("Event type must be set in configuration in standalone mode");
      return StatusCode::FAILURE;
    }
    else if (m_DAQMode == ZdcEventInfo::MCDigits || eventInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) {
      eventType = ZdcEventInfo::ZdcSimulation;
    }
    else if ((m_DAQMode == ZdcEventInfo::PhysicsPEB) || (m_DAQMode == ZdcEventInfo::CombinedPhysics)) {
      if (eventInfo->eventType(xAOD::EventInfo::IS_CALIBRATION)) {
	//
	// For now presume LED event. Eventually add check for calreq trigger
	//
	eventType = ZdcEventInfo::ZdcEventLED;
      }
      else {
	eventType = ZdcEventInfo::ZdcEventPhysics;
      }
    } 
  }

  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleContainer (m_zdcModuleContainerName, ctx);
  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleSumContainer (m_zdcSumContainerName, ctx);

  
  // Find the sum container for "side 0" which handles event-level information
  //
  for (const auto modSum : *(moduleSumContainer.get())) {
    //
    // Module sum object with side == 0 contains event-level information
    //
    if (modSum->zdcSide() == 0) {
      //
      // Add the event type and daq mode as aux decors, but not if this information is already written (e.g. if we are using an existing xAOD)
      //
      static const SG::ConstAccessor<unsigned int> eventTypeAcc("EventType");
      if (!eventTypeAcc.isAvailable(*modSum))
	{ 
	  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> eventTypeHandle(m_ZdcEventType);
	  eventTypeHandle(*modSum) = eventType;
	  
	  SG::WriteDecorHandle<xAOD::ZdcModuleContainer,unsigned int> DAQModeHandle(m_ZdcDAQMode);
	  DAQModeHandle(*modSum) = m_DAQMode;
	}
    }
  }

  
  // Loop over all tools and perform the "reco" 
  //
  for (ToolHandle<ZDC::IZdcAnalysisTool>& tool : m_zdcTools)
    {
      ATH_CHECK( tool->recoZdcModules(*moduleContainer.get(), *moduleSumContainer.get()) );
    }

  return StatusCode::SUCCESS;

}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecRun3::finalize()
{

  ATH_MSG_DEBUG( "--> ZDC: ZdcRecRun3 finalize complete" );

  return StatusCode::SUCCESS;

}
//==================================================================================================

