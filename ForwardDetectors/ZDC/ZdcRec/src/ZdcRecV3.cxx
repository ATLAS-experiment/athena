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

#include "xAODForward/ZdcModuleContainer.h"
#include "ZdcRec/ZdcRecV3.h"
#include "ZdcAnalysis/ZdcAnalysisTool.h"

//==================================================================================================
ZdcRecV3::ZdcRecV3(const std::string& name, ISvcLocator* pSvcLocator) :

	AthAlgorithm(name, pSvcLocator),
	m_ownPolicy(static_cast<int> (SG::OWN_ELEMENTS))
{
	declareProperty("OwnPolicy",m_ownPolicy) ;
}
//==================================================================================================

//==================================================================================================
ZdcRecV3::~ZdcRecV3() {}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3::initialize()
{
	MsgStream mLog(msgSvc(), name());

	// Reconstruction Tool
	ATH_CHECK( m_zdcTool.retrieve() );

	ATH_CHECK( m_zdcModuleContainerName.initialize() );
	ATH_CHECK( m_zdcSumContainerName.initialize() );

	if (m_ownPolicy == SG::OWN_ELEMENTS)
		mLog << MSG::DEBUG << "...will OWN its cells." << endmsg;
	else
		mLog << MSG::DEBUG << "...will VIEW its cells." << endmsg;


	mLog << MSG::DEBUG << "--> ZDC: ZdcRecV3 initialization complete" << endmsg;

	return StatusCode::SUCCESS;
}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3::execute(const EventContext& ctx)
{

  ATH_MSG_DEBUG ("--> ZDC: ZdcRecV3 execute starting on "
                 << ctx.evt()
                 << "th event");

  
  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleContainer (m_zdcModuleContainerName, ctx);
  SG::ReadHandle<xAOD::ZdcModuleContainer> moduleSumContainer (m_zdcSumContainerName, ctx);

  ATH_CHECK( m_zdcTool->recoZdcModules(*moduleContainer.get(), *moduleSumContainer.get() ) ); // passes by reference

  return StatusCode::SUCCESS;

}
//==================================================================================================

//==================================================================================================
StatusCode ZdcRecV3::finalize()
{

  ATH_MSG_DEBUG( "--> ZDC: ZdcRecV3 finalize complete" );

  return StatusCode::SUCCESS;

}
//==================================================================================================

