/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONLAYERSURFACE_H
#define MUON_MUONLAYERSURFACE_H

#include "MuonStationIndex/MuonStationIndex.h"
#include "TrkSurfaces/Surface.h"
#include "memory"

namespace Muon {

    /** types */
    struct MuonLayerSurface {
        using SurfacePtr = std::shared_ptr<const Trk::Surface>;
        using DetRegIdx = MuonStationIndex::DetectorRegionIndex;
        using LayerIdx = MuonStationIndex::LayerIndex;
        MuonLayerSurface() = default;

        MuonLayerSurface(SurfacePtr surfacePtr_, int sector_, DetRegIdx regionIndex_,
                         LayerIdx layerIndex_) :
            surfacePtr{surfacePtr_}, sector{sector_}, regionIndex{regionIndex_}, layerIndex{layerIndex_} {}

        inline MuonStationIndex::StIndex stIndex() const {
            return Muon::MuonStationIndex::toStationIndex(regionIndex, layerIndex);
        }
        SurfacePtr surfacePtr{nullptr};
        int sector{-1};
        DetRegIdx regionIndex{DetRegIdx::DetectorRegionUnknown};
        LayerIdx layerIndex{LayerIdx::LayerUnknown};
    };

}  // namespace Muon

#endif
