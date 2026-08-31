/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
  */

#include "src/RandomProtoTrackCreatorTool.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"


ActsTrk::RandomProtoTrackCreatorTool::RandomProtoTrackCreatorTool(const std::string& type, 
								  const std::string& name,
								  const IInterface* parent)
  : base_class(type,name,parent)
{}

StatusCode ActsTrk::RandomProtoTrackCreatorTool::findProtoTracks(const EventContext& ctx,
								 const xAOD::PixelClusterContainer & pixelContainer,
								 const xAOD::StripClusterContainer & stripContainer,
								 std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const {
    // Sample N random hits for example
    ATH_MSG_VERBOSE("Generate a random proto track in "<<ctx.eventID()<<" from "<<pixelContainer.size()<<" pixel measurements "
                    <<", "<<stripContainer.size()<<" strip measurements");
    std::vector<const xAOD::UncalibratedMeasurement*> dummyPoints;  
    size_t nPix = 1; 
    size_t nStrip = 7; 
    for (size_t k = 0; k < nPix; ++k){
        //we don't worry about the quality of randomness for this purpose
        //coverity[DC.WEAK_CRYPTO)]
        auto index = rand() % pixelContainer.size();
        dummyPoints.push_back(pixelContainer.at(index));
    }


    for (size_t k = 0; k < nStrip; ++k){
        //coverity[DC.WEAK_CRYPTO)]
        auto index = rand() % stripContainer.size();
        dummyPoints.push_back(stripContainer.at(index));
    }

    ATH_MSG_DEBUG("Made a proto-track with " <<dummyPoints.size()<<" random clusters");


    // Make the intput perigee
    auto inputPerigee = makeDummyParams(dummyPoints[0]);

    // and add to the list (will only make one prototrack per event for now)
    foundProtoTracks.push_back({std::move(dummyPoints),std::move(inputPerigee)});

    return StatusCode::SUCCESS;
}

Amg::Vector3D ActsTrk::RandomProtoTrackCreatorTool::getMeasurementPos(const xAOD::UncalibratedMeasurement* theMeas) const {
    if (theMeas->type() == xAOD::UncalibMeasType::PixelClusterType) {
      return static_cast <const xAOD::PixelCluster*>(theMeas)->globalPosition().cast<double>();
    } else if (theMeas->type() == xAOD::UncalibMeasType::StripClusterType){
      return static_cast<const xAOD::StripCluster*>(theMeas)->globalPosition().cast<double>();
    }
    return Amg::Vector3D::Zero();
}


std::unique_ptr<Acts::BoundTrackParameters> ActsTrk::RandomProtoTrackCreatorTool::makeDummyParams (const xAOD::UncalibratedMeasurement*  measurement) const{

  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
        Acts::Vector3::Zero());
  Acts::BoundVector params{};

  auto globalPos = getMeasurementPos(measurement); 

  // No, this is not a physically correct parameter estimate! 
  // We just want a placeholder to point in roughly the expected direction... 
  // A real track finder would do something more reasonable here. 
  params << 0., 0.,
        globalPos.phi(), globalPos.theta(),
        1. / (1000000000. * 1_MeV), 0.;
 

  // Covariance - let's be honest and say we have no clue ;-) 
  Acts::BoundMatrix cov = Acts::BoundMatrix::Identity();
  cov *= 100000; 

  return std::make_unique<Acts::BoundTrackParameters>(actsSurface, params,
                                    cov,  Acts::ParticleHypothesis::pion());

}
