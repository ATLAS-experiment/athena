/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkVertexWeightCalculators/JetRestrictedSumPtVertexWeightCalculator.h"
#include "TrkParameters/TrackParameters.h"
#include "VxVertex/VxTrackAtVertex.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/Vertex.h"
#include "xAODJet/Jet.h"

#include <vector>
#include <set>
#include <iomanip>

namespace Trk {

StatusCode
JetRestrictedSumPtVertexWeightCalculator::initialize()
{
  ATH_CHECK(m_jetContKey.initialize(!m_jetContKey.empty()));
  ATH_CHECK(m_tracksInCone.retrieve());
  
  return StatusCode::SUCCESS;
} // end of initialize method

StatusCode
JetRestrictedSumPtVertexWeightCalculator::finalize()
{
  return StatusCode::SUCCESS;
}

// class constructor implementation
JetRestrictedSumPtVertexWeightCalculator::JetRestrictedSumPtVertexWeightCalculator(
  const std::string& t,
  const std::string& n,
  const IInterface* p)
  : AthAlgTool(t, n, p)
{
  declareInterface<IVertexWeightCalculator>(this);
}

double
JetRestrictedSumPtVertexWeightCalculator::estimateSignalCompatibility(
  const xAOD::Vertex& vertex) const
{
  //::First get the jet containers
  SG::ReadHandle<xAOD::JetContainer> jetContHandle(m_jetContKey);
  if (!jetContHandle.isValid()){  
    ATH_MSG_ERROR ("HardScatterSelectionTool configured to use jet collection "<<m_jetContKey.key()<<", but collection is not found!"); 
  }
  const xAOD::JetContainer* jetCont{jetContHandle.cptr()};

  // Retrieve tracks within jet cone (cached lookup)
  // Add to a set for comparison later
  std::set<const xAOD::TrackParticle*> tracks_in_jets;
  for(const xAOD::Jet* jet : *jetCont) {
    std::vector<const xAOD::TrackParticle*> tracks_in_this_jet;
    m_tracksInCone->particlesInCone( jet->eta(), jet->phi(), m_cone_dR, tracks_in_this_jet );
    tracks_in_jets.insert(tracks_in_this_jet.begin(),tracks_in_this_jet.end());
  }
  ATH_MSG_VERBOSE("Collected " << tracks_in_jets.size() << " tracks in " << jetCont->size() << " jets");

  size_t n_selected_tracks{0};
  double total_pt{0.}, jet_only_pt{0.};
  ATH_MSG_DEBUG("Estimating vertex sorting score from "
                << vertex.nTrackParticles() << " tracks at vertex.");
  for (const auto& elTrackParticle : vertex.trackParticleLinks()) {

    if (not elTrackParticle.isValid()) {
      ATH_MSG_WARNING(
        "No valid link to tracks in xAOD::Vertex object. Skipping track for "
        "signal compatibility (may be serious).");
      continue;
    }

    const Trk::Perigee& perigee =
      (*elTrackParticle.cptr())->perigeeParameters();
    float increment = 0.;
    if (m_doSumPt2Selection) {
      increment = std::pow(1. / perigee.parameters()[Trk::qOverP] *
                             sin(perigee.parameters()[Trk::theta]) / 1000.,
                           2);
    } else {
      increment = std::fabs(1. / perigee.parameters()[Trk::qOverP]) *
                  sin(perigee.parameters()[Trk::theta]) / 1000.;
    }
    total_pt += increment;

    if(tracks_in_jets.contains(*elTrackParticle)) {
      ATH_MSG_VERBOSE("Accepted track with index " << (*elTrackParticle)->index() << " in jet vicinity.");
      ++n_selected_tracks;
      jet_only_pt += increment;
    }

  }
  ATH_MSG_VERBOSE("Counted " << n_selected_tracks << "/" << vertex.nTrackParticles()
    << " towards vertex sumpt " << std::setprecision(3) << jet_only_pt << " (unrestricted sum: " << total_pt << ")");
  return total_pt;
}

} /// End!!!

