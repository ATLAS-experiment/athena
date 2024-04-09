/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Package : NSWRawDataMonAlg
// Authors:   M. Biglietti (Roma Tre)
//
// DESCRIPTION:
// Subject: NSW-->Offline Muon Data Quality
/////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "NSWDataMonAlg.h"

/////////////////////////////////////////////////////////////////////////////
// *********************************************************************
// Public Methods
// ********************************************************************* 

NSWDataMonAlg::NSWDataMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
	AthMonitorAlgorithm(name,pSvcLocator)
{ }

/*---------------------------------------------------------*/
StatusCode NSWDataMonAlg::initialize()
/*---------------------------------------------------------*/
{
	//init message stream
	ATH_MSG_DEBUG("initialize NSWDataMonAlg");
	ATH_MSG_DEBUG("******************");
	ATH_MSG_DEBUG("doNSWESD: " << m_doESD );
	ATH_MSG_DEBUG("******************");

	ATH_CHECK(AthMonitorAlgorithm::initialize());
	ATH_CHECK(m_muonKey.initialize());


	ATH_MSG_DEBUG(" end of initialize " );
	ATH_MSG_INFO("NSWDataMonAlg initialization DONE " );

	return StatusCode::SUCCESS;
} 

StatusCode NSWDataMonAlg::fillHistograms(const EventContext& ctx) const
{

	ATH_MSG_DEBUG("NSWDataMonAlg::MM RawData Monitoring Histograms being filled" );
	 SG::ReadHandle<xAOD::MuonContainer> muonContainer(m_muonKey, ctx);
	 if (!muonContainer.isValid()) {
	   ATH_MSG_FATAL("Could not get muon container: " << m_muonKey.fullKey());
	   return StatusCode::FAILURE;
	 }


	return StatusCode::SUCCESS;
}

