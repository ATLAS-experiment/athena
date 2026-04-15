/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTOOLINTERFACES_IPROTOTRACKCREATORTOOL__H
#define ACTSTOOLINTERFACES_IPROTOTRACKCREATORTOOL__H

// Athena 
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "BeamSpotConditionsData/BeamSpotData.h"
#include "Acts/MagneticField/MagneticFieldContext.hpp"

// ACTS EDM
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"
#include "ActsEvent/ProtoTrack.h"

#include "Acts/Definitions/Algebra.hpp"

namespace ActsTrk {
  class IProtoTrackCreatorTool
    : virtual public IAlgTool {
  public:
    DeclareInterfaceID(IProtoTrackCreatorTool, 1, 0);
    
    /// @brief EF-style pattern recognition to create prototracks 
    /// @param ctx: Event context
    /// @param pixelContainer: pixel cluster 
    /// @param stripContainer: sct cluster 
    /// @param foundProtoTracks: vector to hold the found proto tracks - will be populated by the method.
    /// Method will not discard existing content 
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const = 0;
  };
  
} // namespace 

#endif 

