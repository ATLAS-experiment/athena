/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONREGIONHOUGH_H
#define MUONREGIONHOUGH_H

#include <cmath>
#include <iostream>
#include <vector>
#include <format>

#include "MuonLayerHough/MuonLayerHough.h"
#include "MuonLayerHough/MuonPhiLayerHough.h"
#include "MuonStationIndex/MuonStationIndex.h"

namespace MuonHough {

    /** class managing geometry of the Hough spaces */
    class MuonDetectorDescription {
    public:
        
        using DetRegIdx = Muon::MuonStationIndex::DetectorRegionIndex;
        using LayIdx = Muon::MuonStationIndex::LayerIndex;
        /// constructor
        MuonDetectorDescription();
        

        RegionDescriptor getDescriptor(int sector, DetRegIdx region, LayIdx layer) const;

    private:
        /// initialize default geometry
        void initDefaultRegions();

        /// cached geometry
        RegionDescriptionVec m_regionDescriptions;  /// region descriptions
    };

    /** class managing all precision Hough transforms in a sector */
    class MuonSectorHough {
    public:
        /// constructor for a given sector using the default geometry
        MuonSectorHough(int sector, const MuonDetectorDescription& regionDescriptions);

        /// destructor
        ~MuonSectorHough();
        
        using DetRegIdx = Muon::MuonStationIndex::DetectorRegionIndex;
        using LayIdx = Muon::MuonStationIndex::LayerIndex;
        /// access the Hough transform for a given region
        MuonLayerHough& hough(DetRegIdx region, LayIdx layer) {
            using namespace Muon::MuonStationIndex;
            int index = sectorLayerHash(region, layer);
            return *m_transforms.at(index);
        }

        /// reset histograms
        void reset();

    private:
        std::vector<std::unique_ptr<MuonLayerHough>> m_transforms;  /// Hough transforms for all regions
                                                    // int m_sector;                                /// sector number
    };

    /** class managing all Hough transforms in the detector */
    class MuonDetectorHough {
    public:
        using DetRegIdx = Muon::MuonStationIndex::DetectorRegionIndex;
        using LayIdx = Muon::MuonStationIndex::LayerIndex;
        /// access phi transform
        MuonPhiLayerHough& phiHough(DetRegIdx region) {
             return *m_phiTransforms[static_cast<int>(region)];
        }

        /// access precision transform
        MuonLayerHough& hough(int sector, DetRegIdx region, LayIdx layer) {
            return m_sectors.at(sector - 1)->hough(region, layer);
        }

        /// reset histograms
        void reset();

        /// constructor using default region definitions
        MuonDetectorHough();

        /// constructor using custom region definitions
        MuonDetectorHough(const RegionDescriptionVec& regionDescriptors);

        /// destructor
        ~MuonDetectorHough();

    private:
        void init();

        std::vector<std::unique_ptr<MuonSectorHough>> m_sectors;          /// sector transforms
        std::vector<std::unique_ptr<MuonPhiLayerHough>> m_phiTransforms;  /// phi transforms
    };

    
}  // namespace MuonHough
#endif
