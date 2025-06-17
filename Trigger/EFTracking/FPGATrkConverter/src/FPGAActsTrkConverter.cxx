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
                  const std::vector<std::vector<FPGATrackSimHit>>& hitsInRoads,
                  const std::vector<FPGATrackSimRoad>& roads) const {

    ATH_MSG_INFO("Creating Acts proto-tracks from FPGA roads...");

    if (hitsInRoads.size() > 0) {
      std::multimap<xAOD::DetectorIDHashType, const xAOD::PixelCluster*> pixelClusterMap;
      for (const xAOD::PixelCluster* cluster : pixelContainer) {
        pixelClusterMap.emplace(cluster->identifierHash(), cluster);
      }

      std::multimap<IdentifierHash, const xAOD::StripCluster*> stripClusterMap;
      for (const xAOD::StripCluster* cluster : stripContainer) {
        stripClusterMap.emplace(cluster->identifierHash(), cluster);
      
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
  std::multimap<xAOD::DetectorIDHashType, const xAOD::PixelCluster*> pixelClusterMap;
  for (const xAOD::PixelCluster* cluster : pixelContainer) {
    pixelClusterMap.emplace(cluster->identifierHash(), cluster);
  }

  std::multimap<IdentifierHash, const xAOD::StripCluster*> stripClusterMap;
  for (const xAOD::StripCluster* cluster : stripContainer) {
    stripClusterMap.emplace(cluster->identifierHash(), cluster);
  }
  for (const FPGATrackSimTrack& track : tracks) {
    if (not track.passedOR()) continue;
    std::vector<ActsTrk::ATLASUncalibSourceLink> points;
    const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();
    ATH_CHECK(findPrototrackMeasurements(ctx, pixelContainer, stripContainer, pixelClusterMap, stripClusterMap, points, hits));
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
                                                             const std::multimap<xAOD::DetectorIDHashType, const xAOD::PixelCluster*> & pixelClusterMap,
                                                              const std::multimap<IdentifierHash, const xAOD::StripCluster*> & stripClusterMap,
                                                             std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements,
                                                             const std::vector <FPGATrackSimHit>& hits) const {
  if (hits.empty()) {
  ATH_MSG_ERROR("Found FPGATrack without hits");
  return StatusCode::FAILURE;
  }

  for (const FPGATrackSimHit& h : hits) {
    if (h.isReal()) {
      if (h.isPixel()) {
        ATH_MSG_DEBUG("Looking for Pixel cluster to match");
        auto range = pixelClusterMap.equal_range(h.getIdentifierHash());
        for (auto it = range.first; it != range.second; ++it) {
          ATH_CHECK(matchTrackMeasurements<xAOD::PixelCluster>(ctx, *(it->second), h, measurements, pixelContainer));
        }
      }
      else if (h.isStrip()) {
        ATH_MSG_DEBUG("Looking for Strip cluster to match");
        auto range = stripClusterMap.equal_range(h.getIdentifierHash());
        for (auto it = range.first; it != range.second; ++it) {
          ATH_CHECK(matchTrackMeasurements<xAOD::StripCluster>(ctx, *(it->second), h, measurements, stripContainer));
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

std::vector<Identifier> FPGAActsTrkConverter::getRdoIdList(const FPGATrackSimHit& hit) const
{
    std::vector<Identifier> ids;
    const auto& idHashVec = hit.getIDHashVec();
    const auto& phiIndexVec = hit.getPhiIndexVec();
    const auto& etaIndexVec = hit.getEtaIndexVec();
    
    ids.reserve(idHashVec.size());

    if (hit.isPixel())
    {
        for (size_t i = 0; i < idHashVec.size(); ++i)
        {
            ids.emplace_back(m_pixelId->pixel_id(
                m_pixelId->wafer_id(idHashVec[i]),
                phiIndexVec[i],
                etaIndexVec[i]
            ));
        }
    }
    else if (hit.isStrip())
    {
        for (size_t i = 0; i < idHashVec.size(); ++i)
        {
            ids.emplace_back(m_SCTId->strip_id(
                m_SCTId->wafer_id(idHashVec[i]),
                phiIndexVec[i]
            ));
        }
    }
    return ids;
}


template <typename XAOD_CLUSTER>
StatusCode FPGAActsTrkConverter::matchTrackMeasurements(const EventContext& ctx,
                                                        const XAOD_CLUSTER& cluster,
                                                        const FPGATrackSimHit & trackHit,
                                                        std::vector<ActsTrk::ATLASUncalibSourceLink>& measurements,
                                                        const DataVector<XAOD_CLUSTER>& clusterContainer) const
{
  std::vector<Identifier> rdoIDs = getRdoIdList(trackHit);
  const auto& rdoList = cluster.rdoList();
  if (rdoIDs.size() != rdoList.size()) return StatusCode::SUCCESS;
  size_t matchedCounter = 0;
  for (const Identifier& id : rdoIDs) {
    if (std::find(rdoList.begin(), rdoList.end(), id) != rdoList.end()) matchedCounter++;
  }

  if (matchedCounter == rdoList.size()) {
    measurements.emplace_back(ActsTrk::makeATLASUncalibSourceLink(&clusterContainer, cluster.index(), ctx));
  }
  else if (matchedCounter > 0) {
    std::stringstream ss;
    ss << "List of xAOD cluster rdoIDs:\n";
    for (const Identifier& id : rdoList) ss << id << "\n";

    ATH_MSG_ERROR(std::format("Noticed rdoID mismatch: commonHits = {} | xAODClusterHits = {} | FPGAClusterHits = {}\n{}",
      matchedCounter, rdoList.size(), rdoIDs.size(), ss.str()));
    return StatusCode::SUCCESS; // TODO: revert to FAILURE once this is fixed
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

