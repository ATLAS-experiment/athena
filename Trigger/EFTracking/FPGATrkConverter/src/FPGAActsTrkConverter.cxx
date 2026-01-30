// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "FPGAActsTrkConverter.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "Identifier/IdentifierHash.h"

#include <format>
#include <sstream>
//Heavily inspired by https://gitlab.cern.ch/atlas/athena/-/blob/main/Tracking/Acts/ActsTrackReconstruction/src/RandomProtoTrackCreator.cxx

FPGAActsTrkConverter::FPGAActsTrkConverter(const std::string& type, 
		const std::string& name,
		const IInterface* parent): base_class(type,name,parent) { }


StatusCode FPGAActsTrkConverter::initialize() {

  ATH_MSG_DEBUG("Initializing FPGAActsTrkConverter...");

  // Get SCT & pixel Identifier helpers
  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_SCTId, "SCT_ID"));

  return StatusCode::SUCCESS;

}

StatusCode FPGAActsTrkConverter::findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks,
                  const FPGATrackSimHitContainer& hitsInRoads,
                  const std::vector<FPGATrackSimRoad>& roads) const {

    ATH_MSG_INFO("Creating Acts proto-tracks from FPGA roads...");

    if (hitsInRoads.size() > 0) {
      std::multimap<xAOD::DetectorIdentType, const xAOD::PixelCluster*> pixelClusterMap;
      for (const xAOD::PixelCluster* cluster : pixelContainer) {
        pixelClusterMap.emplace(cluster->identifier(), cluster);
      }

      std::multimap<xAOD::DetectorIdentType, const xAOD::StripCluster*> stripClusterMap;
      for (const xAOD::StripCluster* cluster : stripContainer) {
        stripClusterMap.emplace(cluster->identifier(), cluster);
      
      }
      for(size_t roadIndex=0; roadIndex<=hitsInRoads.size()-1;roadIndex++) { 
        std::vector<ActsTrk::ATLASUncalibSourceLink> points;  
        ATH_CHECK(findPrototrackMeasurements(ctx, pixelContainer, stripContainer, pixelClusterMap, stripClusterMap, points, hitsInRoads.at(roadIndex)));
        if (points.size()) {
          std::unique_ptr<Acts::BoundTrackParameters> inputPerigee = makeParams(roads.at(roadIndex));
          foundProtoTracks.emplace_back(points, std::move(inputPerigee));
          ATH_MSG_INFO("Made a prototrack with " << points.size() << " measurements");
        }
      }
    }

    return StatusCode::SUCCESS;
}

StatusCode FPGAActsTrkConverter::findProtoTracks(const EventContext& ctx,
                                                 const xAOD::PixelClusterContainer& pixelContainer,
                                                 const xAOD::StripClusterContainer& stripContainer,
                                                 std::vector<ActsTrk::ProtoTrack>& foundProtoTracks,
                                                 const std::vector<FPGATrackSimTrack>& tracks) const {

  ATH_MSG_INFO("Creating Acts proto-tracks from FPGA tracks...");
  // Initialize multimaps for pixel and strip clusters
  std::multimap<xAOD::DetectorIdentType, const xAOD::PixelCluster*> pixelClusterMap;
  for (const xAOD::PixelCluster* cluster : pixelContainer) {
    pixelClusterMap.emplace(cluster->identifier(), cluster);
  }

  std::multimap<xAOD::DetectorIdentType, const xAOD::StripCluster*> stripClusterMap;
  for (const xAOD::StripCluster* cluster : stripContainer) {
    stripClusterMap.emplace(cluster->identifier(), cluster);
  }
  for (const FPGATrackSimTrack& track : tracks) {
    if (not track.passedOR()) continue;
    std::vector<ActsTrk::ATLASUncalibSourceLink> points;
    const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();
    auto hitCollection = std::make_unique<FPGATrackSimHitCollection>();
    hitCollection->reserve(hits.size());
    for (const auto& hit : hits) {
      hitCollection->push_back(new FPGATrackSimHit(hit));
    }
    ATH_CHECK(findPrototrackMeasurements(ctx, pixelContainer, stripContainer, pixelClusterMap, stripClusterMap, points, *hitCollection));
    if (points.size()) {
      ATH_MSG_DEBUG("\tMaking a proto-track with " << points.size() << " clusters");
      std::unique_ptr<Acts::BoundTrackParameters> inputPerigee = makeParams(track);
      foundProtoTracks.emplace_back(points, std::move(inputPerigee));
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode FPGAActsTrkConverter::findPrototrackMeasurements( const EventContext& ctx,
                                                             const xAOD::PixelClusterContainer& pixelContainer,
                                                             const xAOD::StripClusterContainer& stripContainer,
                                                             const std::multimap<xAOD::DetectorIdentType, const xAOD::PixelCluster*> & pixelClusterMap,
                                                              const std::multimap<xAOD::DetectorIdentType, const xAOD::StripCluster*> & stripClusterMap,
                                                             std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements,
                                                             const FPGATrackSimHitCollection& hits) const {
  if (hits.empty()) {
  ATH_MSG_ERROR("Found FPGATrack without hits");
  return StatusCode::FAILURE;
  }

  for (const FPGATrackSimHit* h : hits) {
    if (h->isReal()) {
      if (h->isPixel()) {
        ATH_MSG_DEBUG("Looking for Pixel cluster to match");
        auto range = pixelClusterMap.equal_range(h->getRdoIdentifier());
        for (auto it = range.first; it != range.second; ++it) {
          ATH_CHECK(matchTrackMeasurements<xAOD::PixelCluster>(ctx, *(it->second), *h, measurements, pixelContainer));
        }
      }
      else if (h->isStrip()) {
        ATH_MSG_DEBUG("Looking for Strip cluster to match");
        auto range = stripClusterMap.equal_range(h->getRdoIdentifier());
        for (auto it = range.first; it != range.second; ++it) {
          ATH_CHECK(matchTrackMeasurements<xAOD::StripCluster>(ctx, *(it->second), *h, measurements, stripContainer));
        }
      }
      else {
        ATH_MSG_ERROR("FPGA hit not classified as pixel or strip");
        return StatusCode::FAILURE;
      }
    }
    else {
      ATH_MSG_DEBUG("Skipping hit as non-Real");
    }
  }
  return StatusCode::SUCCESS;
}


template <typename XAOD_CLUSTER>
StatusCode FPGAActsTrkConverter::matchTrackMeasurements(const EventContext& ctx,
                                                        const XAOD_CLUSTER& cluster,
                                                        const FPGATrackSimHit & trackHit,
                                                        std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements,
                                                        const DataVector<XAOD_CLUSTER>& clusterContainer) const
{
  const Identifier::value_type trackId = (trackHit.getHitType() == HitType::spacepoint) 
                                          ? trackHit.getOriginalHit().getRdoIdentifier() 
                                          : trackHit.getRdoIdentifier();

  if(cluster.identifier() == trackId) {
    measurements.emplace_back(ActsTrk::makeATLASUncalibSourceLink(&clusterContainer, cluster.index(), ctx));
    ATH_MSG_DEBUG("Matched FPGATrackSimHit to xAOD cluster");
  }

  return StatusCode::SUCCESS;
}

std::unique_ptr<Acts::BoundTrackParameters> FPGAActsTrkConverter::makeParams (const FPGATrackSimRoad &road) const{
  using namespace Acts::UnitLiterals;

  std::shared_ptr<const Acts::Surface> actsSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0., 0., 0.));
  Acts::BoundVector params;

  constexpr double GeVToMeV = 1000;
  double d0=0.; //?
  double z0=0.; //?
  double phi=road.getX();
  double eta=0.2;
  double theta=2*std::atan(std::exp(-eta));
  double qop = (std::abs(road.getY()) > 1E-9) ? road.getY()/GeVToMeV : 1E-12;
  double t=0.; //?
  ATH_MSG_DEBUG("\tphi=" <<phi << " eta=" << eta << " qop=" << qop);

  params << d0, z0, phi, theta, qop, t; 

  // Covariance - TODO
  Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Identity();
  cov *= (GeVToMeV*GeVToMeV); 

  // some ACTS paperwork 
  Trk::ParticleHypothesis hypothesis = Trk::pion;
  float mass = Trk::ParticleMasses::mass[hypothesis] * Acts::UnitConstants::MeV;
  Acts::PdgParticle absPdg = Acts::makeAbsolutePdgParticle(Acts::ePionPlus);
  Acts::ParticleHypothesis actsHypothesis{
    absPdg, mass, Acts::AnyCharge{1.0f}};

  return std::make_unique<Acts::BoundTrackParameters>(actsSurface, params,
                                    cov, actsHypothesis);

}


std::unique_ptr<Acts::BoundTrackParameters> FPGAActsTrkConverter::makeParams (const FPGATrackSimTrack &track) const{

  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(Acts::Vector3(0., 0., 0.));
  Acts::BoundVector params;

  constexpr double GeVToMeV = 1000.;
  double d0=track.getD0();
  double z0=track.getZ0();
  double phi=track.getPhi();
  double eta=track.getEta();
  double theta=track.getTheta();
  double qopt=track.getQOverPt()*GeVToMeV;
  double pt=track.getPt()/GeVToMeV;
  double px=pt*std::cos(phi);
  double py=pt*std::sin(phi);
  double pz=pt*std::sinh(eta);
  double p=std::sqrt(px*px+py*py+pz*pz);
  double qop=((p > 1e-10) ? (1/p) : 1e10);
  if (qopt < 0) qop *= -1;  
  double t=0.;

  params << d0, z0, phi, theta, qop, t;  
  ATH_MSG_DEBUG("\td0= " << d0 << " z0=" <<z0 << " phi=" <<phi << " theta=" << theta<< " qoverp=" << qop);

  // Covariance - let's be honest and say we have no clue ;-) 
  Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Identity();
  
  (cov)(0,0) *= 0.16; // d0: 0.4 **2 (conservative)
  (cov)(1,1) *= 25; // z0: 5**2 = 25 (conservative)
  (cov)(2,2) *= 0.0008; // phi: 0.02**2 = 0.0004, increase a bit = double
  (cov)(3,3) *= 0.0008; // width in eta is nearly 0.2, but width in theta = 2*atan(e^-eta) will vary. Take biggest one, which is at eta of 0 when width is 0.02, so get 0.02**2 = 0.0004, increase a bit = double
  (cov)(4,4) *= 0.36; // qop also varies with eta. Error on q/pt conservatively = 0.0003 in mev ^-1, or 0.3 in gev ^-1, double to start giving us 0.6. then square that to get 0.36


    
  // some ACTS paperwork 
  Trk::ParticleHypothesis hypothesis = Trk::pion;
  float mass = Trk::ParticleMasses::mass[hypothesis] * Acts::UnitConstants::MeV;
  Acts::PdgParticle absPdg = Acts::makeAbsolutePdgParticle(Acts::ePionPlus);
  Acts::ParticleHypothesis actsHypothesis{
    absPdg, mass, Acts::AnyCharge{1.0f}};

  return std::make_unique<Acts::BoundTrackParameters>(actsSurface, params,
                                    cov, actsHypothesis);

}

