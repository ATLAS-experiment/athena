/*
   Copyright (C) 2002-2026  CERN for the benefit of the ATLAS collaboration
 */

#include "ActsEgammaSelectedTrackCopy.h"
//
#include "AthenaKernel/errorcheck.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "FourMomUtils/P4Helpers.h"
#include "GaudiKernel/EventContext.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "egammaUtils/CandidateMatchHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "Acts/Geometry/TrackingGeometry.hpp"
#include "Acts/Surfaces/CurvilinearSurface.hpp"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Utilities/Zip.hpp"
// std includes
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

using xAOD::EgammaHelpers::summaryValueInt;

ActsEgammaSelectedTrackCopy::ActsEgammaSelectedTrackCopy(const std::string& name,
                                                         ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
ActsEgammaSelectedTrackCopy::initialize()
{
  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_clusterContainerKey.initialize());
  ATH_CHECK(m_OutputTrkPartContainerKey.initialize());
  ATH_CHECK(m_egammaCaloClusterSelector.retrieve());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());
  ATH_CHECK(m_extrapolationTool->initialize());
  ATH_CHECK(m_trackingGeometryTool.retrieve());

  // Here we extract the geometry identifiers of the 4 calo volumes in the ACTS geometry
  // It seems more robust to do the matching with the volume name, since the geometry ID
  // can change if details of the geometry change.
  const static std::map<std::string, std::size_t> volIndex = {
    {"PreSamplerB_Layer", 0},
    {"EMB1_Layer", 1},
    {"EMB2_Layer", 2},
    {"EMB3_Layer", 3}
  };

  // Debug info in case volume matching fails
  std::vector<std::string> unmatchedVolumes;

  m_trackingGeometryTool->trackingGeometry()->visitVolumes([&](const Acts::TrackingVolume *vol) {
    auto name = vol->volumeName();
    if( volIndex.contains(name) ) {
      ATH_MSG_DEBUG(vol->volumeName() << " - " << vol->geometryId() << " - surfaces: " << vol->surfaces().size());
      m_barrelCaloGeoIds.at(volIndex.at(name)) = vol->geometryId();
    } else {
      unmatchedVolumes.push_back(name);
    }
  });

  // Check that all geoIds were found
  for(const auto &[geoId, layerInfo] : Acts::zip(m_barrelCaloGeoIds, volIndex)) {
    if( geoId == Acts::GeometryIdentifier{} ) {
      ATH_MSG_ERROR("Could not find geometry ID for barrel calo layer '" << layerInfo.first << "'");

      ATH_MSG_ERROR("List of unmatched volumes:");
      std::stringstream ss;
      for( const auto &name : unmatchedVolumes ) {
        ss << name << ", ";
      }
      ATH_MSG_ERROR(ss.str());

      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode
ActsEgammaSelectedTrackCopy::finalize()
{
  ATH_MSG_INFO("===> ACTS egamma Selected Tracks Statistics ======");
  ATH_MSG_INFO("--- All Central Clusters: " << m_AllClusters);
  ATH_MSG_INFO("--- Selected Central Clusters: " << m_SelectedClusters);
  ATH_MSG_INFO("--- All Tracks: " << m_AllTracks);
  ATH_MSG_INFO("--- Selected Central Tracks: " << m_SelectedTracks);
  ATH_MSG_INFO("<=================================================");

  return StatusCode::SUCCESS;
}

StatusCode
ActsEgammaSelectedTrackCopy::execute(const EventContext& ctx) const
{
  SG::ReadHandle<xAOD::CaloClusterContainer> clusterTES = SG::makeHandle(m_clusterContainerKey, ctx);
  ATH_CHECK(clusterTES.isValid());
  m_AllClusters += clusterTES->size();

  SG::ReadHandle<xAOD::TrackParticleContainer> trackTES = SG::makeHandle(m_trackParticleContainerKey, ctx);
  ATH_CHECK(trackTES.isValid());
  m_AllTracks += trackTES->size();

  // Here it just needs to be a view copy , i.e the collection of selected
  // trackParticles we create does not really own its elements.
  using output_collection_t = ConstDataVector<xAOD::TrackParticleContainer>;
  SG::WriteHandle< output_collection_t > outputTrkPartContainer = SG::makeHandle(m_OutputTrkPartContainerKey, ctx);
  ATH_CHECK( outputTrkPartContainer.record( std::make_unique<output_collection_t>(SG::VIEW_ELEMENTS) ) );
  output_collection_t *viewCopy = outputTrkPartContainer.ptr();

  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle = SG::makeHandle(m_caloDetDescrMgrKey, ctx);
  ATH_CHECK(caloDetDescrMgrHandle.isValid());

  const CaloDetDescrManager* calodetdescrmgr = caloDetDescrMgrHandle.cptr();

  // Perigee surface at the origin. TODO: use beamspot conditions data.
  auto perigeeSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3::Zero());

  // Check which clusters to seed on.
  std::vector<const xAOD::CaloCluster*> passingClusters;
  passingClusters.reserve(clusterTES->size());
  for (const xAOD::CaloCluster* cluster : *clusterTES) {
    if (m_egammaCaloClusterSelector->passSelection(cluster, *calodetdescrmgr)) {
      passingClusters.push_back(cluster);
    }
  }

  if( passingClusters.empty() ) {
    ATH_MSG_DEBUG("No cluster left after selection");
    return StatusCode::SUCCESS;
  }

  m_SelectedClusters += passingClusters.size();

  for (const xAOD::TrackParticle* track : *trackTES) {
    // For now only barrel
    if( std::abs(track->eta()) > s_calorimeterEtaCoverage ) {
      ATH_MSG_DEBUG("Track eta " << track->eta() << " outside coverage " <<
                    "|" << s_calorimeterEtaCoverage << "|");
      continue;
    }


    for (const xAOD::CaloCluster* cluster : passingClusters) {
      // First we will see if it fails the quick match.
      // Then if it passed it will get 2 chances to be selected.
      // One if it matches from last measurement.
      // The second if it matched from Perigee rescales.
      if( !checkBroadCriteria(*cluster, *track) ) {
        ATH_MSG_DEBUG("Track fails broad criteria");
        continue;
      }

      if (matchWithExtrapolation(ctx, *cluster, *track, perigeeSurface)) {
        viewCopy->push_back(track);
        break;
      }
    }
  }

  m_SelectedTracks += viewCopy->size(); 

  return StatusCode::SUCCESS;
}

bool
ActsEgammaSelectedTrackCopy::matchWithExtrapolation(const EventContext& ctx,
                                                    const xAOD::CaloCluster& cluster,
                                                    const xAOD::TrackParticle& track,
                                                    const std::shared_ptr<const Acts::Surface>& perigeeSurface) const
{
  using namespace Acts::UnitLiterals;

  ATH_MSG_VERBOSE("Extrapolating track to calo cluster at position: "
                  << cluster.eta() << ", " << cluster.phi());

  // Extrapolate from last measurement to the four EM layers.
  unsigned int lastMeasIdx = 0;
  if (!track.indexOfParameterAtPosition(lastMeasIdx, xAOD::LastMeasurement)) {
    ATH_MSG_WARNING("TrackParticle has no last measurement parameters");
    return false;
  }

  Acts::Vector3 lastPos{track.parameterX(lastMeasIdx),
                        track.parameterY(lastMeasIdx),
                        track.parameterZ(lastMeasIdx)};
  Acts::Vector3 lastMom{track.parameterPX(lastMeasIdx),
                        track.parameterPY(lastMeasIdx),
                        track.parameterPZ(lastMeasIdx)};
  lastMom *= Acts::UnitConstants::MeV;

  ATH_MSG_DEBUG("Last measurement global position: " << lastPos.transpose());

  Acts::BoundVector lastBoundParams = Acts::BoundVector::Zero();
  lastBoundParams[Acts::eBoundPhi]    = Acts::VectorHelpers::phi(lastMom);
  lastBoundParams[Acts::eBoundTheta]  = Acts::VectorHelpers::theta(lastMom);
  lastBoundParams[Acts::eBoundQOverP] = track.charge() / lastMom.norm();

  std::shared_ptr<const Acts::Surface> lastSurface =
    Acts::CurvilinearSurface(lastPos, lastMom.normalized()).planeSurface()->getSharedPtr();
  Acts::BoundTrackParameters boundPars{
    lastSurface,
    lastBoundParams,
    std::nullopt,
    Acts::ParticleHypothesis::electron()
  };

  // First try the EMB2, if this does not match, print a message and try any other calo layer
  auto extractCaloMatch = [&](const auto &caloMatches) {
    if( caloMatches.at(2).has_value() ) {
      return caloMatches.at(2);;
    }
    ATH_MSG_DEBUG("No calo match at EM2, trying any other layer");
    for( const auto &matchOpt : caloMatches ) {
      if( matchOpt.has_value() ) {
        ATH_MSG_DEBUG("Using calo match at layer " << matchOpt->layer);
        return matchOpt;
      }
    }
    return std::optional<CaloMatch>{};
  };

  auto caloMatches = extrapolateToCalo(boundPars, cluster, ctx);
  auto caloMatch = extractCaloMatch(caloMatches);
  if( !caloMatch.has_value() ) {
    ATH_MSG_DEBUG("Could not extrapolate to calo from last measurement");
    return false;
  }

  // Get the calo match at EM2.
  ATH_MSG_DEBUG("Calo match from last measurement at " << caloMatch->layer
                << ", dEta: " << caloMatch->deltaEta
                << ", dPhi: " << caloMatch->deltaPhi);

  // First check if delta eta is OK, fail if not (rescaling should not affect eta).
  if (std::abs(caloMatch->deltaEta) > m_narrowDeltaEta) {
    ATH_MSG_DEBUG("Fails narrow window eta match, deta=" << caloMatch->deltaEta);
    return false;
  }

  // Selection in narrow phi window from last measurement.
  if ( caloMatch->deltaPhi >= -m_narrowDeltaPhiBrem &&
       caloMatch->deltaPhi <= m_narrowDeltaPhi ) {
    ATH_MSG_DEBUG("Match from Last measurement is successful, deta=" << caloMatch->deltaEta
                  << " , dphi=" << caloMatch->deltaPhi);
    return true;
  }

  // Now try the rescale from perigee
  Acts::BoundVector paramsAtPerigee = Acts::BoundVector::Zero();
  paramsAtPerigee[Acts::eBoundLoc0]  = track.d0();
  paramsAtPerigee[Acts::eBoundLoc1]  = track.z0();
  paramsAtPerigee[Acts::eBoundPhi]   = track.phi0();
  paramsAtPerigee[Acts::eBoundTheta] = track.theta();
  paramsAtPerigee[Acts::eBoundQOverP] = track.charge() / (cluster.e() * Acts::UnitConstants::MeV);

  Acts::BoundTrackParameters rescaledParsPerigee{
    perigeeSurface,
    paramsAtPerigee,
    std::nullopt,
    Acts::ParticleHypothesis::electron()
  };

  const auto caloMatchesRescaled = extrapolateToCalo(rescaledParsPerigee, cluster, ctx);
  auto caloMatchRescaled = extractCaloMatch(caloMatchesRescaled);

  if( !caloMatchRescaled.has_value() ) {
    ATH_MSG_DEBUG("Could not extrapolate to calo from rescaled perigee");
    return false;
  }

  // Get the calo match at EM2.
  ATH_MSG_DEBUG("Calo match from perigee rescaled at " << caloMatchRescaled->layer
                << ", dEta: " << caloMatchRescaled->deltaEta
                << ", dPhi: " << caloMatchRescaled->deltaPhi);

  // Selection in narrow phi window
  if ( caloMatchRescaled->deltaPhi >= -m_narrowRescaleBrem &&
       caloMatchRescaled->deltaPhi <= m_narrowRescale ) {
    ATH_MSG_DEBUG("Match from Perigee Rescaled is successful, dphi=" << caloMatchRescaled->deltaPhi);
    return true;
  }

  ATH_MSG_DEBUG("Match not successful");
  return false;
}

bool
ActsEgammaSelectedTrackCopy::checkBroadCriteria(const xAOD::CaloCluster& cluster,
                                                const xAOD::TrackParticle& track) const
{
  const Trk::Perigee& perigee = track.perigeeParameters();
  
  // Get Perigee Parameters.
  const double trkPhi = perigee.parameters()[Trk::phi];
  const double z_perigee = perigee.position().z();
  const Amg::Vector3D PerigeeXYZPosition(perigee.position().x(),
                                         perigee.position().y(),
                                         z_perigee);

  // Get Cluster parameters.
  const double clusterEta = xAOD::EgammaHelpers::isFCAL(&cluster) ? cluster.eta() : cluster.etaBE(2);
  const bool isEndCap = !xAOD::EgammaHelpers::isBarrel(&cluster);

  // Use perigee.eta() only if sufficient hits in the Si.
  const double Et = cluster.e() / std::cosh(perigee.eta());

  ATH_MSG_VERBOSE("clusterEta: " << clusterEta);
  ATH_MSG_VERBOSE("perigee.eta(): " << perigee.eta());
  ATH_MSG_VERBOSE("Et: " << Et);
  ATH_MSG_VERBOSE("isEndCap: " << std::boolalpha << isEndCap);

  // Calculate the eta/phi of the cluster as would be seen from the perigee
  // position of the Track.
  const Amg::Vector3D globalClusterPosWrtPerigee = CandidateMatchHelpers::approxXYZwrtPoint(
    cluster, perigee.position(), isEndCap);

  // Calculate the possible rotation of the track.
  // Once assuming the cluster Et being the better estimate (e.g big brem).
  const double phiRotRescaled = CandidateMatchHelpers::PhiROT(
    Et, perigee.eta(), track.charge(), perigee.position().perp(), isEndCap);

  // And also assuming the track Pt being correct.
  const double phiRotTrack = CandidateMatchHelpers::PhiROT(
    track.pt(), perigee.eta(), track.charge(), perigee.position().perp(), isEndCap);

  const double clusterPhiCorrected = globalClusterPosWrtPerigee.phi();

  // DeltaPhi between the track and the cluster.
  const double deltaPhiStd = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhi);
  const bool failPhiStd = std::abs(deltaPhiStd) > m_broadDeltaPhi;

  // DeltaPhi between the track and the cluster accounting for rotation assuming
  // cluster Et is a better estimator.
  const double trkPhiRescaled = P4Helpers::deltaPhi(trkPhi, phiRotRescaled);
  const double deltaPhiRescaled = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhiRescaled);
  const bool failPhiRescalded = std::abs(deltaPhiRescaled) > m_broadDeltaPhi;

  // DeltaPhi between the track and the cluster accounting for rotation.
  const double trkPhiCorrTrack = P4Helpers::deltaPhi(trkPhi, phiRotTrack);
  const double deltaPhiTrack = P4Helpers::deltaPhi(clusterPhiCorrected, trkPhiCorrTrack);
  const bool failPhiTrack = std::abs(deltaPhiTrack) > m_broadDeltaPhi;

  // Broad phi check.
  if (failPhiRescalded && failPhiTrack && failPhiStd) {
    ATH_MSG_DEBUG(
      "FAILS broad window phi match (track phi, phirotCluster , phiRotTrack , " <<
      "cluster phi corrected, cluster phi): ( " <<
      trkPhi << ", " <<
      phiRotRescaled << ", " <<
      phiRotTrack << ", " <<
      clusterPhiCorrected << ", " <<
      cluster.phi() << ")"
    );
    return false;
  }

  // Broad eta check.
  const double clusterEtaCorrected = globalClusterPosWrtPerigee.eta();
  const bool failEta = std::abs(cluster.eta() - perigee.eta()) > m_broadDeltaEta;
  const bool failEtaCorrected = std::abs(globalClusterPosWrtPerigee.eta() - perigee.eta()) > m_broadDeltaEta;
  
  if (failEta && failEtaCorrected) {
    ATH_MSG_DEBUG(
      "FAILS broad window eta match (track eta, cluster eta, cluster eta corrected): ( " <<
      perigee.eta() << ", " <<
      cluster.eta() << ", " <<
      clusterEtaCorrected << " )"
    );
    return false;
  }

  return true;
}

std::array<std::optional<ActsEgammaSelectedTrackCopy::CaloMatch>, 4> ActsEgammaSelectedTrackCopy::extrapolateToCalo(
  const Acts::BoundTrackParameters &parameters, const xAOD::CaloCluster& cluster,
  const EventContext& ctx) const
{
  auto res = m_extrapolationTool.get()->propagationSteps(ctx, parameters);
  if( !res.ok() ) {
    ATH_MSG_DEBUG("Error during extrapolation: " << res.error().message());
    return {};
  }

  const auto &[steps, _] = res.value();

  std::array<std::optional<CaloMatch>, 4> matches{};
  for(const auto &step : steps) {
    if( step.surface == nullptr || step.surface->geometryId().sensitive() == 0 ) {
      continue;
    }
    const auto &p = step.position;
    if ( std::hypot(p.x(), p.y()) > s_maxExtrapolationRadius ) {
      ATH_MSG_DEBUG("Extrapolated beyond calo radius, stopping.");
      break;
    }

    ATH_MSG_VERBOSE("Extrapolated step at pos [r,z]: " << std::hypot(p.x(), p.y())
                  << ", " << p.z() << " - : " << step.geoID);

    auto found = std::ranges::find_if(m_barrelCaloGeoIds, [&](const Acts::GeometryIdentifier &id){
      return id.volume() == step.geoID.volume();
    });

    if( found == m_barrelCaloGeoIds.end() ) {
      ATH_MSG_VERBOSE("Step not in barrel calo layers");
      continue;
    }
    int layerIdx = std::distance(m_barrelCaloGeoIds.begin(), found);
    assert(layerIdx >= 0 && layerIdx < 4);

    if( matches.at(layerIdx).has_value() ) { 
      ATH_MSG_VERBOSE("Step in calo layer already visited");
      continue;
    }

    auto eta = Acts::VectorHelpers::eta(p);
    auto phi = Acts::VectorHelpers::phi(p);

    double deltaEta = eta - cluster.etaBE(layerIdx);
    double deltaPhi = P4Helpers::deltaPhi(phi, cluster.phiBE(layerIdx));

    ATH_MSG_DEBUG("Step at pos [r,z]: " << std::hypot(p.x(), p.y()) <<
                  ", layerIdx: " << layerIdx <<
                  ", " << p.z() << " - : " << step.geoID <<
                  ", dEta: " << deltaEta << ", dPhi: " << deltaPhi);

    matches.at(layerIdx) = CaloMatch{
      layerIdx,
      eta,
      phi,
      deltaEta,
      deltaPhi
    };
  }

  return matches;
}

