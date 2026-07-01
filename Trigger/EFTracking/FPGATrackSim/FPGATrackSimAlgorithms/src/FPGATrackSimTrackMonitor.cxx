// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
void FPGATrackSimTrackMonitor::fillRoad(const std::vector<FPGATrackSimRoad>& roads,
                                        const std::vector<FPGATrackSimTruthTrack>& truthTracks,
                                        size_t nLogicalLayers)
{
  // overall number of roads
  auto nRoads = Monitored::Scalar<int>("nRoads", roads.size());
  // push them into the monitoring tool (this->MonTool set in python)
  Monitored::Group(m_monTool, nRoads);

  // update counter of total roads
  m_totNElements += roads.size();

  // loop over all roads in the event
  for (const auto& road : roads) {
    // layerIDs
    unsigned bitmask = road.getHitLayers();
    for (size_t l = 0; l < nLogicalLayers; l++) {
      if (bitmask & (1 << l)) {
          auto mon_layerIDs = Monitored::Scalar<unsigned>("layerIDs",l);
          Monitored::Group(m_monTool, mon_layerIDs);
      }
    }
    // number of layers represented in this road
    auto mon_nLayers = Monitored::Scalar<int>("nLayers", road.getNLayers());
    // number of hits
    auto mon_nHits = Monitored::Scalar<int>("nHits", road.getNHits());
    // push all of the above scalars into the monitoring tool
    Monitored::Group(m_monTool, mon_nLayers, mon_nHits);
    // roads with 4 hits
    if (road.getNHits() == 4) {
      // fit chi2 in this road
      auto mon_fitChi2_4 = Monitored::Scalar<float>("chi2_4", road.getFitChi2());
      // fit chi2Eta
      auto mon_fitChi2Eta_4 = Monitored::Scalar<float>("chi2Eta_4", road.getFitChi2Eta());
      // fit chi2Phi
      auto mon_fitChi2Phi_4 = Monitored::Scalar<float>("chi2Phi_4", road.getFitChi2Phi());
      // push all of the above scalars into the monitoring tool
      Monitored::Group(m_monTool, mon_fitChi2_4, mon_fitChi2Eta_4, mon_fitChi2Phi_4);
    } else if (road.getNHits() == 5) {
      auto mon_fitChi2_5 = Monitored::Scalar<float>("chi2_5", road.getFitChi2());
      auto mon_fitChi2Eta_5 = Monitored::Scalar<float>("chi2Eta_5", road.getFitChi2Eta());
      auto mon_fitChi2Phi_5 = Monitored::Scalar<float>("chi2Phi_5", road.getFitChi2Phi());
      Monitored::Group(m_monTool, mon_fitChi2_5, mon_fitChi2Eta_5, mon_fitChi2Phi_5);
    }
  }

  // only monitor efficiency for events with truth tracks
  if (truthTracks.size() > 0) {
    // update road counter
    if (roads.size() > 0) ++m_nElements;
    // update the max number of elements
    if (roads.size() > m_maxNElements) m_maxNElements=roads.size();
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
  // monitor chi2 for all tracks and find the best
  float bestChi2 = chi2Cut;
  float chi2_best = chi2Cut;
  float chi2Eta_best = chi2Cut;
  float chi2Phi_best = chi2Cut;
  float chi2_best_4 = chi2Cut;
  float chi2Eta_best_4 = chi2Cut;
  float chi2Phi_best_4 = chi2Cut;
  float chi2_best_5 = chi2Cut;
  float chi2Eta_best_5 = chi2Cut;
  float chi2Phi_best_5 = chi2Cut;

  // lambda function to monitor chi2 (all) and keep track of the best one
  auto monitorChi2 = [&](const std::string& chi2_name, float chi2_value, float& best_value, std::string_view suffix = {}) {
  if (chi2_value < chi2Cut) {
    // monitor all chi2
    auto mon = Monitored::Scalar<float>(chi2_name + std::string{suffix}, chi2_value);
    Monitored::Group(m_monTool, mon);
    // update best chi2
    if (chi2_value < best_value) best_value = chi2_value;
  }
};


  // overall number of tracks
  auto mon_nTracks = Monitored::Scalar<int>("nTracks", tracks.size());
  // push them into the monitoring tool (this->MonTool set in python)
  Monitored::Group(m_monTool, mon_nTracks);

  // update counter of total tracks
  m_totNElements += tracks.size();

  // loop over all tracks in the event
  for (const auto& track : tracks) {
    // number of real hits
    int nValidHits = 0;
    for (const auto& hit : track->getFPGATrackSimHits()) {
      if (hit.isReal()) {
        ++nValidHits;
      }
    }
    auto mon_nHits = Monitored::Scalar<int>("nHits", nValidHits);
    Monitored::Group(m_monTool, mon_nHits);

    // monitor chi2 for tracks
    float chi2_all = track->getChi2ndof();
    if (chi2_all < chi2Cut) {
      auto mon_chi2_all = Monitored::Scalar<float>("chi2_all", chi2_all);
      Monitored::Group(m_monTool, mon_chi2_all);
      // update best chi2
      if (chi2_all < bestChi2) {
        bestChi2 = chi2_all;
      }
    }
    
    // monitor base chi2 values
    monitorChi2("chi2",    track->getChi2(),    chi2_best);
    monitorChi2("chi2Eta", track->getChi2Eta(), chi2Eta_best);
    monitorChi2("chi2Phi", track->getChi2Phi(), chi2Phi_best);
    
    // monitor nHits-dependent chi2 values
    if (nValidHits == 4) {
      monitorChi2("chi2",    track->getChi2(),    chi2_best_4,    "_4");
      monitorChi2("chi2Eta", track->getChi2Eta(), chi2Eta_best_4, "_4");
      monitorChi2("chi2Phi", track->getChi2Phi(), chi2Phi_best_4, "_4");
    } else if (nValidHits == 5) {
      monitorChi2("chi2",    track->getChi2(),    chi2_best_5,    "_5");
      monitorChi2("chi2Eta", track->getChi2Eta(), chi2Eta_best_5, "_5");
      monitorChi2("chi2Phi", track->getChi2Phi(), chi2Phi_best_5, "_5");
    }
  }
  // monitor the best chi2 found
  if (bestChi2 < chi2Cut) {
    auto mon_best_chi2 = Monitored::Scalar<float>("best_chi2", bestChi2);
    Monitored::Group(m_monTool, mon_best_chi2);
  }

  // lambda function to monitor the best chi2 value
  auto monitorBestChi2 = [&](const std::string& best_name, float best_value) {
    if (best_value < chi2Cut) {
      auto mon_best = Monitored::Scalar<float>(best_name, best_value);
      Monitored::Group(m_monTool, mon_best);
    }
  };

  // best chi2 values
  monitorBestChi2("chi2_best",    chi2_best);
  monitorBestChi2("chi2Eta_best", chi2Eta_best);
  monitorBestChi2("chi2Phi_best", chi2Phi_best);
  monitorBestChi2("chi2_best_4",    chi2_best_4);
  monitorBestChi2("chi2Eta_best_4", chi2Eta_best_4);
  monitorBestChi2("chi2Phi_best_4", chi2Phi_best_4);
  monitorBestChi2("chi2_best_5",    chi2_best_5);
  monitorBestChi2("chi2Eta_best_5", chi2Eta_best_5);
  monitorBestChi2("chi2Phi_best_5", chi2Phi_best_5);


  // only monitor efficiency for events with truth tracks
  if (truthTracks.size() > 0) {
    // update track counter
    if (tracks.size() > 0) ++m_nElements;
    // update the max number of elements
    if (tracks.size() > m_maxNElements) m_maxNElements=tracks.size();
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


