/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigAFPSidHypoMonitoringAlg.h"

#include "xAODForward/AFPTrackContainer.h"
#include "xAODForward/AFPTrack.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include <cmath>

TrigAFPSidHypoMonitoringAlg::TrigAFPSidHypoMonitoringAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthMonitorAlgorithm (name, pSvcLocator)
{
}

TrigAFPSidHypoMonitoringAlg::~TrigAFPSidHypoMonitoringAlg()
{
}

StatusCode TrigAFPSidHypoMonitoringAlg::initialize()
{
  ATH_CHECK(AthMonitorAlgorithm::initialize());
 
  ATH_CHECK(m_AFPtrackKey.initialize());
  ATH_CHECK(m_AFPtrackOffKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode TrigAFPSidHypoMonitoringAlg::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode TrigAFPSidHypoMonitoringAlg::fillHistograms(const EventContext& context) const
{

  SG::ReadHandle<xAOD::AFPTrackContainer> tracksAFP(m_AFPtrackKey, context);

  SG::ReadHandle<xAOD::AFPTrackContainer> tracksAFPoff(m_AFPtrackOffKey, context);

  ATH_MSG_DEBUG("AFP track size online/offline:"<<tracksAFP.get()->size()<<"/"<<tracksAFPoff.get()->size());

  auto xDiff = Monitored::Scalar("xDiff",-999.);
  auto yDiff = Monitored::Scalar("yDiff",-999.);

  // Match online track to offline
  for(const auto* track: *tracksAFP){
    float dRmin = 9e9;
    for(const auto* off_track: *tracksAFPoff){
      
      float dx = track->xLocal()-off_track->xLocal();
      float dy = track->yLocal()-off_track->yLocal();
      float dR = std::hypot(dx, dy);

      if(dR<dRmin){
	dRmin = dR;
	xDiff = dx;
	yDiff = dy;
      }
    } // End of loop over offline tracks

    fill("AFPCoarse", xDiff, yDiff);

  }// End of loop over online tracks

  // in future should be split into many smaller methods called from here
  std::vector<std::string> passedAFPChains = {"all"}; // also includes ALL events counter
  for (const auto& chainName: m_chains) {
    if ( getTrigDecisionTool()->isPassed(chainName) ){
      passedAFPChains.emplace_back(chainName);
    }

  }
  if ( passedAFPChains.size() > 1) passedAFPChains.emplace_back("AFP");
  auto whichTrigger = Monitored::Collection("TrigCounts", passedAFPChains);
  fill("AFPCoarse", whichTrigger);

  return StatusCode::SUCCESS;
}

