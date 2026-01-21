/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFPGAACTSTRKCONVERTER__H
#define IFPGAACTSTRKCONVERTER__H

// Athena 
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"

// ACTS EDM
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "ActsEvent/ProtoTrack.h"

#include "Acts/Definitions/Algebra.hpp"

#include "FPGATrackSimObjects/FPGATrackSimRoad.h"
#include "FPGATrackSimObjects/FPGATrackSimTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimHitContainer.h"


  class IFPGAActsTrkConverter
    : virtual public IAlgTool {
  public:
     
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ,
                  const FPGATrackSimHitContainer & hitsInRoads,
                  const std::vector<FPGATrackSimRoad>& roads) const = 0;
    virtual StatusCode findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ,
                  const std::vector<FPGATrackSimTrack>& tracks) const = 0;
  };
  

#endif 

