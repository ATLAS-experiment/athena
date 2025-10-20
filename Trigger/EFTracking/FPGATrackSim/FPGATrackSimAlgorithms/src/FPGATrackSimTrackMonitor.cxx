// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimGenScanBinning.cxx
 * @author E.L.
 * @date August 08, 2025
 * @brief See header file.
 */

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include <bit>
#include "FPGATrackSimTrackMonitor.h"


//// calls the AthAlgTool contructor
FPGATrackSimTrackMonitor::FPGATrackSimTrackMonitor(const std::string& algname, const std::string &name, const IInterface *ifc) :
  AthAlgTool(algname, name, ifc)
{
  //// logs a message confirming construction
  ATH_MSG_INFO("constructor");
}

//// book histograms
StatusCode FPGATrackSimTrackMonitor::initialize()
{
  // print all configured properties (debugging)
  auto props = this->getProperties();
  for( Gaudi::Details::PropertyBase* prop : props ) {
    if (prop->ownerTypeName()==this->type()) {      
      ATH_MSG_DEBUG("Property:\t" << prop->name() << "\t : \t" << prop->toString());
    }
  }
  ATH_MSG_INFO("Initialize Track Monitoring");

  //// needed to register histograms and confirms initialization
  ATH_CHECK(m_tHistSvc.retrieve());

  return StatusCode::SUCCESS;

}



#include "AthenaMonitoringKernel/Monitored.h"


//// fill monitored roads
void FPGATrackSimTrackMonitor::fillRoad(const std::vector<std::shared_ptr<const FPGATrackSimRoad>>& roads,
                                        const std::vector<FPGATrackSimTruthTrack>& truthTracks,
                                        size_t nLogicalLayers)
{
  // overall number of roads
  auto nRoads = Monitored::Scalar<int>("nRoads", roads.size());
  // push them into the monitoring tool (this->MonTool set in python)
  Monitored::Group(m_monTool, nRoads);

  // return if no roads
  if ( roads.empty() ) return;

  // loop over all roads in the event
  for (const auto& road : roads) {
    // layerIDs
    unsigned bitmask = road->getHitLayers();
    for (size_t l = 0; l < nLogicalLayers; l++) {
      if (bitmask & (1 << l)) {
          auto mon_layerIDs = Monitored::Scalar<unsigned>("layerIDs",l);
          Monitored::Group(m_monTool,mon_layerIDs);
      }
    }
    // number of layers represented in this road
    auto nLayers = Monitored::Scalar<int>("nLayers", road->getNLayers());
    // push all of the above scalars into the monitoring tool
    Monitored::Group(m_monTool, nLayers);
  }

  // only monitor efficiency for events with truth tracks
  if (truthTracks.size() > 0) {
    // use the first truth track (the primary one)
    const auto& truthTrack = truthTracks.front();
    // efficiency monitor
    auto mon_eff = Monitored::Scalar<bool>("eff_road", (roads.size() > 0));
    auto mon_truth_pt_zoom = Monitored::Scalar<float>("pT_zoom", truthTrack.getPt()*0.001);
    auto mon_truth_pt = Monitored::Scalar<float>("pT", truthTrack.getPt()*0.001);
    auto mon_truth_eta = Monitored::Scalar<float>("eta", truthTrack.getEta());
    auto mon_truth_phi = Monitored::Scalar<float>("phi", truthTrack.getPhi());
    auto mon_truth_d0 = Monitored::Scalar<float>("d0", truthTrack.getD0());
    auto mon_truth_z0 = Monitored::Scalar<float>("z0", truthTrack.getZ0());
    Monitored::Group( m_monTool, mon_eff, mon_truth_pt_zoom, mon_truth_pt, mon_truth_eta, mon_truth_phi, mon_truth_d0, mon_truth_z0 );
  }
}


//// fill monitored tracks
void FPGATrackSimTrackMonitor::fillTrack(const std::vector<const FPGATrackSimTrack*>& tracks,
                                          const std::vector<FPGATrackSimTruthTrack>& truthTracks,
                                          float chi2Cut)
{
  // overall number of tracks
  auto nTracks = Monitored::Scalar<int>("nTracks", tracks.size());
  // push them into the monitoring tool (this->MonTool set in python)
  Monitored::Group(m_monTool, nTracks);

  // return if no tracks
  if ( tracks.empty() ) return;

  // monitor chi2 for all tracks and find the best
  float bestChi2 = chi2Cut;
  // loop over all tracks in the event
  for (const auto& track : tracks) {
    float chi2 = track->getChi2ndof();
    // only monitor tracks below chi2 cut
    if (chi2 < chi2Cut) {
      auto mon_chi2 = Monitored::Scalar<float>("chi2_all", chi2);
      Monitored::Group(m_monTool, mon_chi2);
      // update best chi2
      if (chi2 < bestChi2) {
        bestChi2 = chi2;
      }
    }
  }
  // monitor the best chi2 found
  if (bestChi2 < chi2Cut) {
    auto mon_best_chi2 = Monitored::Scalar<float>("best_chi2", bestChi2);
    Monitored::Group(m_monTool, mon_best_chi2);
  }

  // only monitor efficiency for events with truth tracks
  if (truthTracks.size() > 0) {
    // use the first truth track (the primary one)
    const auto& truthTrack = truthTracks.front();
    // efficiency monitor
    auto mon_eff = Monitored::Scalar<bool>("eff_track", (tracks.size() > 0));
    auto mon_truth_pt_zoom = Monitored::Scalar<float>("pT_zoom", truthTrack.getPt()*0.001);
    auto mon_truth_pt = Monitored::Scalar<float>("pT", truthTrack.getPt()*0.001f);
    auto mon_truth_eta = Monitored::Scalar<float>("eta", truthTrack.getEta());
    auto mon_truth_phi = Monitored::Scalar<float>("phi", truthTrack.getPhi());
    auto mon_truth_d0 = Monitored::Scalar<float>("d0", truthTrack.getD0());
    auto mon_truth_z0 = Monitored::Scalar<float>("z0", truthTrack.getZ0());
    Monitored::Group( m_monTool, mon_eff, mon_truth_pt_zoom, mon_truth_pt, mon_truth_eta, mon_truth_phi, mon_truth_d0, mon_truth_z0 );
  }
}


