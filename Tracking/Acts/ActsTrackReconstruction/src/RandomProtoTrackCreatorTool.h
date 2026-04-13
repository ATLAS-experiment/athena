/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACKRECONSTRUCTION_RANDOMPROTOTRACKCREATOR__H
#define ACTSTRACKRECONSTRUCTION_RANDOMPROTOTRACKCREATOR__H

#include "ActsToolInterfaces/IProtoTrackCreatorTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsGeometry/ATLASSourceLink.h"

namespace ActsTrk {

  class RandomProtoTrackCreatorTool :
    public extends<AthAlgTool, ActsTrk::IProtoTrackCreatorTool> {
  public:
    
    RandomProtoTrackCreatorTool(const std::string& type, 
				const std::string& name,
				const IInterface* parent);
    virtual ~RandomProtoTrackCreatorTool() = default;
    
      /// @brief EF-style pattern recognition to create prototracks 
      /// @param ctx: Event context
      /// @param pixelContainer: pixel cluster 
      /// @param stripContainer: sct cluster 
      /// @param foundProtoTracks: vector to hold the found proto tracks - will be populated by the method.
      /// Method will not discard existing content 
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainerAlt & pixelContainer,
                  const xAOD::StripClusterContainerAlt & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const override final; 
    protected:
    /// @brief creates a random, dummy set of parameters 
    /// Warning: This is not a real parameter estimate. Should only serve as a placeholder. Use with care
    /// @param firstPRD: First hit on our proto track
    /// @return a set of dummy params - just pointing from the origin in a straight line to our hit
    std::unique_ptr<Acts::BoundTrackParameters> makeDummyParams (const ActsTrk::ATLASUncalibSourceLink & firstPRD) const;
    /// @brief get the global position for an uncalibrated measurement - delegates to the specialisation
    /// @param theMeas: uncalibrated measurement
    Amg::Vector3D getMeasurementPos(const xAOD::UncalibratedMeasurement* theMeas) const; 

  };

} // namespace

#endif



