/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONCHAMBERLAYERDESCRIPTION_H
#define MUON_MUONCHAMBERLAYERDESCRIPTION_H

#include <vector>

#include "MuonDetDescrUtils/MuonChamberLayerDescriptor.h"

namespace Muon {

    /** class managing geometry of the chamber layers */
    class MuonChamberLayerDescription {
    public:
        /// constructor
        MuonChamberLayerDescription();

        using LayerIdx = MuonStationIndex::LayerIndex;
        using DetRegIdx = MuonStationIndex::DetectorRegionIndex;

        MuonChamberLayerDescriptor getDescriptor(int sector, DetRegIdx region, LayerIdx layer) const;

    private:
        /// initialize default geometry
        void initDefaultRegions();

        /// cached geometry
        using MuonChamberLayerDescriptorVec =  std::vector<MuonChamberLayerDescriptor>;
        MuonChamberLayerDescriptorVec m_chamberLayerDescriptors{};  /// region descriptions
    };
}  // namespace Muon

#endif
