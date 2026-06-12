/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 * ZdcRecV3.cxx
 *
 *  Created on: Sept 11, 2016 (never forget)
 *      Author: Peter.Steinberg@bnl.gov
 */


#include <memory>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/StatusCode.h"
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
//#include "Identifier/Identifier.h"

#include "xAODForward/ZdcModuleAuxContainer.h"
#include "xAODForward/ZdcModuleContainer.h"
#include "ZdcRec/ZdcRecV3Decode.h"
#include "ZdcRec/ZdcRecChannelToolV2.h"
#include "xAODForward/ZdcModuleToString.h"
#include "ZdcByteStream/ZdcToString.h"

//==================================================================================================
ZdcRecV3Decode::ZdcRecV3Decode(const std::string& name, ISvcLocator* pSvcLocator) :

	AthAlgorithm(name, pSvcLocator),
	m_ownPolicy(static_cast<int> (SG::OWN_ELEMENTS))
{
	declareProperty("OwnPolicy",m_ownPolicy) ;
}
//==================================================================================================

//==================================================================================================
ZdcRecV3Decode::~ZdcRecV3Decode() {}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3Decode::initialize()
{
	MsgStream mLog(msgSvc(), name());

	// Unpacking tool
	ATH_CHECK( m_ChannelTool.retrieve() );


	ATH_CHECK( m_zdcModuleContainerName.initialize() );
	ATH_CHECK( m_zdcSumContainerName.initialize() );
	ATH_CHECK( m_ttContainerName.initialize(SG::AllowEmpty) );

	if (m_ownPolicy == SG::OWN_ELEMENTS)
		mLog << MSG::DEBUG << "...will OWN its cells." << endmsg;
	else
		mLog << MSG::DEBUG << "...will VIEW its cells." << endmsg;


	mLog << MSG::DEBUG << "--> ZDC: ZdcRecV3Decode initialization complete" << endmsg;

	return StatusCode::SUCCESS;
}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3Decode::execute(const EventContext& ctx)
{

  ATH_MSG_DEBUG ("--> ZDC: ZdcRecV3Decode execute starting on "
                 << ctx.evt()
                 << "th event");

  //Look for the container presence
  if (m_ttContainerName.empty()) {
    return StatusCode::SUCCESS;
  }

  // Look up the Digits "TriggerTowerContainer" in Storegate
  SG::ReadHandle<xAOD::TriggerTowerContainer> ttContainer (m_ttContainerName, ctx);
  
  //Create the containers to hold the reconstructed information (you just pass the pointer and the converter does the work)	
  std::unique_ptr<xAOD::ZdcModuleContainer> moduleContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> moduleAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  moduleContainer->setStore( moduleAuxContainer.get() );

  //Create the containers to hold the reconstructed information (you just pass the pointer and the converter does the work)	
  std::unique_ptr<xAOD::ZdcModuleContainer> moduleSumContainer( new xAOD::ZdcModuleContainer());
  std::unique_ptr<xAOD::ZdcModuleAuxContainer> moduleSumAuxContainer( new xAOD::ZdcModuleAuxContainer() );
  moduleSumContainer->setStore( moduleSumAuxContainer.get() );
  
  // rearrange ZDC channels and perform fast reco on all channels (including non-big tubes)
  int ncha = m_ChannelTool->convertTT2ZM(ttContainer.get(), moduleContainer.get(), moduleSumContainer.get() );
  ATH_MSG_DEBUG("m_ChannelTool->convertTT2ZM returned " << ncha << " channels");
  //msg( MSG::DEBUG ) << ZdcModuleToString(*moduleContainer) << endmsg;
  
  SG::WriteHandle<xAOD::ZdcModuleContainer> moduleContainerH (m_zdcModuleContainerName, ctx);
  ATH_CHECK( moduleContainerH.record (std::move(moduleContainer),
				      std::move(moduleAuxContainer)) );

  SG::WriteHandle<xAOD::ZdcModuleContainer> sumsContainerH (m_zdcSumContainerName, ctx);
  ATH_CHECK( sumsContainerH.record (std::move(moduleSumContainer),
				      std::move(moduleSumAuxContainer)) );

  return StatusCode::SUCCESS;

}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3Decode::finalize()
{

  ATH_MSG_DEBUG( "--> ZDC: ZdcRecV3Decode finalize complete" );

  return StatusCode::SUCCESS;

}
//==================================================================================================

