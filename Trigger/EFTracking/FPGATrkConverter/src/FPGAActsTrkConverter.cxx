// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "FPGAActsTrkConverter.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "ActsGeometry/ATLASSourceLink.h"
#include "Identifier/IdentifierHash.h"

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

      for(size_t roadIndex=0; roadIndex<=hitsInRoads.size()-1;roadIndex++) { 
        std::vector<ActsTrk::ATLASUncalibSourceLink> points;  

        if (hitsInRoads.at(roadIndex).size()>0) {
        // TODO: move from loops over cluster container to links 
          for(const FPGATrackSimHit& h : hitsInRoads.at(roadIndex)){
            IdentifierHash hash = h.getIdentifierHash();
            if (h.isReal()) {
              if (h.isPixel()){
                ATH_MSG_DEBUG("Looking for Pixel cluster to match");
                Identifier wafer_id = m_pixelId->wafer_id(hash);
                Identifier id = m_pixelId->pixel_id(wafer_id, h.getPhiIndex(), h.getEtaIndex()); 
                for (const xAOD::PixelCluster* cl : pixelContainer) {
                  const auto& rdoList = cl->rdoList();
                  const auto it = std::find(rdoList.begin(), rdoList.end(), id);
                  if (it != rdoList.end()) {
                    points.emplace_back(ActsTrk::makeATLASUncalibSourceLink(&pixelContainer, cl->index(), ctx));
                    break;
                  }
                }
              }
              if (h.isStrip()) {
                ATH_MSG_DEBUG("Looking for Strip cluster to match");
                int strip = static_cast<int>(h.getPhiCoord());
                Identifier wafer_id = m_SCTId->wafer_id(hash);
                Identifier id = m_SCTId->strip_id(wafer_id, strip);
                for (const xAOD::StripCluster *cl : stripContainer){
                  const auto& rdoList = cl->rdoList();
                  const auto it = std::find(rdoList.begin(), rdoList.end(), id);
                  if (it != rdoList.end()) {
                    points.push_back(ActsTrk::makeATLASUncalibSourceLink(&stripContainer, cl->index(), ctx)); 
                    break;}
                }
              }
            }
          }
          if(!points.size()){
            ATH_MSG_ERROR("Found a proto-track with no measurements");
            continue; // TODO: once resolved, instead of simply discarding these cases, return StatusCode::FAILURE
          }
          else {
            ATH_MSG_INFO("\tMade a proto-track with " << points.size() << " clusters");

            // Make the input perigee
            std::unique_ptr<Acts::BoundTrackParameters> inputPerigee = makeParams(roads.at(roadIndex));
            foundProtoTracks.emplace_back(points, std::move(inputPerigee));
          }
        }
      }
    }

    return StatusCode::SUCCESS;
}

StatusCode FPGAActsTrkConverter::findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks,
                  const std::vector<FPGATrackSimTrack>& tracks) const {
    
    ATH_MSG_INFO("Creating Acts proto-tracks from FPGA tracks...");

    for (const FPGATrackSimTrack& track : tracks) {
      if (not track.passedOR()) continue;
      std::vector<ActsTrk::ATLASUncalibSourceLink> points;
      const std::vector <FPGATrackSimHit>& hits = track.getFPGATrackSimHits();

      if (hits.size() > 0) {

        // TODO: move from loops over cluster container to links 

        for (const FPGATrackSimHit& h : hits) {
          IdentifierHash hash = h.getIdentifierHash();
          if (h.isReal()) {
            if (h.isPixel()) {
              ATH_MSG_DEBUG("Looking for Pixel cluster to match");
              Identifier wafer_id = m_pixelId->wafer_id(hash);
              Identifier id = m_pixelId->pixel_id(wafer_id, h.getPhiIndex(), h.getEtaIndex());
              for (const xAOD::PixelCluster* cl : pixelContainer) {
                const auto& rdoList = cl->rdoList();
                const auto it = std::find(rdoList.begin(), rdoList.end(), id);
                if (it != rdoList.end()) { 
                  points.emplace_back(ActsTrk::makeATLASUncalibSourceLink(&pixelContainer, cl->index(), ctx)); 
                  break;}
              }
            }
            else if (h.isStrip()) {
              ATH_MSG_DEBUG("Looking for Strip cluster to match");
              int strip = static_cast<int>(h.getPhiCoord());
              Identifier wafer_id = m_SCTId->wafer_id(hash);
              Identifier id = m_SCTId->strip_id(wafer_id, strip);
              for (const xAOD::StripCluster* cl : stripContainer) {
                const auto& rdoList = cl->rdoList();
                const auto it = std::find(rdoList.begin(), rdoList.end(), id);
                if (it != rdoList.end()) {
                  points.push_back(ActsTrk::makeATLASUncalibSourceLink(&stripContainer, cl->index(), ctx));
                  break;
                }
              }
            }
            else {
              ATH_MSG_ERROR("FPGA hit not clasified as pixel or strip");
              return StatusCode::FAILURE;
            }
          }
          else
            ATH_MSG_DEBUG("Skipping hit as non-Real");
        }
        if (!points.size()){
          ATH_MSG_ERROR("Found a proto-track with no measurements");
          continue; // TODO: once resolved, instead of simply discarding these cases, return StatusCode::FAILURE
        }
        else {
          ATH_MSG_INFO("\tMade a proto-track with " << points.size() << " clusters");

          // Make the intput perigee
          std::unique_ptr<Acts::BoundTrackParameters> inputPerigee = makeParams(track);
          foundProtoTracks.emplace_back(points, std::move(inputPerigee));
        }
      }
      else {
        ATH_MSG_ERROR("Found FPGATrack without hits");
        return StatusCode::FAILURE;
      }
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
  double qop=road.getY()/GeVToMeV; //
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

  constexpr double GeVToMeV = 1000;
  double d0=track.getD0();
  double z0=track.getZ0();
  double phi=track.getPhi();
  double theta=track.getTheta();
  double qop=track.getQOverPt();
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

