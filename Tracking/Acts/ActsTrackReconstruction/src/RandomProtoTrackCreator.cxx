/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */
#include "RandomProtoTrackCreator.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"


ActsTrk::RandomProtoTrackCreator::RandomProtoTrackCreator(const std::string& type, 
		const std::string& name,
		const IInterface* parent): base_class(type,name,parent){
}
StatusCode ActsTrk::RandomProtoTrackCreator::findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const {
    // Sample N random hits for example
    std::vector<ActsTrk::ATLASUncalibSourceLink> dummyPoints;  
    size_t nPix = 1; 
    size_t nStrip = 7; 
    for (size_t k = 0; k < nPix; ++k){
        auto index = rand() % pixelContainer.size();
        dummyPoints.push_back(makeATLASUncalibSourceLink(&pixelContainer, index, ctx));
    }


    for (size_t k = 0; k < nStrip; ++k){
        auto index = rand() % stripContainer.size();
        dummyPoints.push_back(makeATLASUncalibSourceLink(&stripContainer, index, ctx));
    }

    ATH_MSG_DEBUG("Made a proto-track with " <<dummyPoints.size()<<" random clusters");


    // Make the intput perigee
    auto inputPerigee = makeDummyParams(dummyPoints[0]);

    // and add to the list (will only make one prototrack per event for now)
    foundProtoTracks.push_back({dummyPoints,std::move(inputPerigee)});

    return StatusCode::SUCCESS;
}

Amg::Vector3D ActsTrk::RandomProtoTrackCreator::getMeasurementPos(const xAOD::UncalibratedMeasurement* theMeas) const{
    if (theMeas->type() == xAOD::UncalibMeasType::PixelClusterType) {
      return dynamic_cast <const xAOD::PixelCluster*>(theMeas)->globalPosition().cast<double>();
    } else if (theMeas->type() == xAOD::UncalibMeasType::StripClusterType){
      return dynamic_cast<const xAOD::StripCluster*>(theMeas)->globalPosition().cast<double>();
    }
    return Amg::Vector3D::Zero();
}


std::unique_ptr<Acts::BoundTrackParameters> ActsTrk::RandomProtoTrackCreator::makeDummyParams (const ActsTrk::ATLASUncalibSourceLink & firstPRD) const{

  const xAOD::UncalibratedMeasurement* measurement = &getUncalibratedMeasurement(firstPRD);
  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
        Acts::Vector3(0., 0., 0.));
  Acts::BoundVector params;

  auto globalPos = getMeasurementPos(measurement); 

  // No, this is not a physically correct parameter estimate! 
  // We just want a placeholder to point in roughly the expected direction... 
  // A real track finder would do something more reasonable here. 
  params << 0., 0.,
        globalPos.phi(), globalPos.theta(),
        1. / (1000000000. * 1_MeV), 0.;
 

  // Covariance - let's be honest and say we have no clue ;-) 
  Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Identity();
  cov *= 100000; 

  // some ACTS paperwork 
  Trk::ParticleHypothesis hypothesis = Trk::pion;
  float mass = Trk::ParticleMasses::mass[hypothesis] * Acts::UnitConstants::MeV;
  Acts::PdgParticle absPdg = Acts::makeAbsolutePdgParticle(Acts::ePionPlus);
  Acts::ParticleHypothesis actsHypothesis{
    absPdg, mass, Acts::AnyCharge{1.0f}};

  return std::make_unique<Acts::BoundTrackParameters>(actsSurface, params,
                                    cov, actsHypothesis);

}
