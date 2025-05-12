/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONCHAMBERLAYERDESCRIPTOR_H
#define MUON_MUONCHAMBERLAYERDESCRIPTOR_H

#include "MuonStationIndex/MuonStationIndex.h"

namespace Muon {

    /** struct containing all information to build a Hough transform for a given chamber index */
    struct MuonChamberLayerDescriptor {
        using DetRegIdx = Muon::MuonStationIndex::DetectorRegionIndex;
        using ChIdx = Muon::MuonStationIndex::ChIndex;

        MuonChamberLayerDescriptor(int sector_, 
                                   DetRegIdx region_,
                                   ChIdx chIndex_, 
                                   float referencePosition_, 
                                   float yMinRange_, 
                                   float yMaxRange_,
                                   float yBinSize_, 
                                   float thetaStep_, 
                                   unsigned int nthetaSamples_) :
            sector(sector_),
            region(region_),
            chIndex(chIndex_),
            referencePosition(referencePosition_),
            yMinRange(yMinRange_),
            yMaxRange(yMaxRange_),
            yBinSize(yBinSize_),
            thetaStep(thetaStep_),
            nthetaSamples(nthetaSamples_) {}
        MuonChamberLayerDescriptor() = default;

        int sector{0};

        DetRegIdx region{DetRegIdx::DetectorRegionUnknown};
        ChIdx chIndex{ChIdx::ChUnknown};
        float referencePosition{0.f};
        float yMinRange{0.f};
        float yMaxRange{0.f};
        float yBinSize{1.f};
        float thetaStep{1.f};
        unsigned int nthetaSamples{1};
    };

}  // namespace Muon

#endif
