/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonLayerHough/MuonRegionHough.h"


namespace MuonHough {    
    using ChIdx = Muon::MuonStationIndex::ChIndex;
    MuonSectorHough::MuonSectorHough(int sector, const MuonDetectorDescription& regionDescriptions) {
        m_transforms.resize(Muon::MuonStationIndex::sectorLayerHashMax());

        // loop over regions and layers of the detector and build the transforms
        constexpr int detRegMax = static_cast<int>(DetRegIdx::DetectorRegionIndexMax);
        constexpr int layIdxMax = static_cast<int>(LayIdx::LayerIndexMax);
        for (int reg = 0; reg < detRegMax; ++reg) {
            const auto region = static_cast<DetRegIdx>(reg);
            for (int lay = 0; lay < layIdxMax; ++lay) {
                const auto layer = static_cast<LayIdx>(lay);

                // skip the few empty slots in the hash
                RegionDescriptor descriptor = regionDescriptions.getDescriptor(sector, region, layer);
                if (descriptor.chIndex == ChIdx::ChUnknown) continue;

                int index = Muon::MuonStationIndex::sectorLayerHash(region, layer);
                m_transforms[index] = std::make_unique<MuonLayerHough>(descriptor);
            }
        }
    }

    MuonSectorHough::~MuonSectorHough() = default;

    void MuonSectorHough::reset() {
        for (auto& transform : m_transforms) {
            if (transform) transform->reset();
        }
    }

    void MuonDetectorHough::reset() {
        for (auto& sector : m_sectors) sector->reset();
        for (auto& transform : m_phiTransforms) transform->reset();
    }

    MuonDetectorHough::MuonDetectorHough() { init(); }

    /// destructor
    MuonDetectorHough::~MuonDetectorHough()  = default;

    void MuonDetectorHough::init() {
        MuonDetectorDescription detectorDescription;  // initialize all the regions
        for (unsigned int i = 1; i <= 16; ++i) { m_sectors.push_back(std::make_unique<MuonSectorHough>(i, detectorDescription)); }
        for (int i = 0; i < static_cast<int>(DetRegIdx::DetectorRegionIndexMax); ++i) {
            m_phiTransforms.push_back(std::make_unique<MuonPhiLayerHough>(60, -M_PI, M_PI, static_cast<DetRegIdx>(i)));
        }
    }

    MuonDetectorDescription::MuonDetectorDescription() { initDefaultRegions(); }

    RegionDescriptor MuonDetectorDescription::getDescriptor(int sector, DetRegIdx region, LayIdx layer) const {
        bool isSmall = (sector % 2 == 0);
        ChIdx chIndex = Muon::MuonStationIndex::toChamberIndex(region, layer, isSmall);
        if (chIndex <= ChIdx::ChUnknown || chIndex >= ChIdx::ChIndexMax) {
            return RegionDescriptor{};
        }

        RegionDescriptor descriptor = m_regionDescriptions[static_cast<int>(chIndex)];
        descriptor.sector = sector;
        // exceptions for a few barrel regions
        if (region == DetRegIdx::Barrel) {
            if ((sector == 10 || sector == 14) && layer == LayIdx::Inner)
                descriptor.referencePosition = 5400.;
            else if ((sector == 11 || sector == 13) && layer == LayIdx::Outer)
                descriptor.referencePosition = 10650.;
        } else if (region == DetRegIdx::EndcapC) {  // multiply reference position by -1 for C side
            descriptor.region = region;
            if (layer == LayIdx::BarrelExtended) {
                descriptor.yMinRange *= -1;
                descriptor.yMaxRange *= -1;
                std::swap(descriptor.yMinRange, descriptor.yMaxRange);
            } else {
                descriptor.referencePosition *= -1;
            }
        }
        return descriptor;
    }

    void MuonDetectorDescription::initDefaultRegions() {
        double scalefactor = 1.0;               // can be used to tune the steps in theta variation!
        int inner_step = 3;                     // default is 3
        int middle_step = 5 * scalefactor;      // default is 5--range is 0.25
        int outer_step = 7 * scalefactor;       // default is 7--range is 0.35
        double inner_gap = 0.05;                // default is 0.05
        double middle_gap = 0.1 / scalefactor;  // default is 0.1
        double outer_gap = middle_gap;   // default is 0.1
        int ystep = 30;                         // default is 30
        m_regionDescriptions.resize(static_cast<int>(ChIdx::ChIndexMax));
        m_regionDescriptions[static_cast<int>(ChIdx::BIS)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BIS,
                                                                              4560, -7500, 7500, ystep, middle_gap, inner_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BIL)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BIL,
                                                                             4950, -7000, 7000, ystep, middle_gap, inner_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BMS)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BMS,
                                                                             8096, -9500, 9500, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BML)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BML,
                                                                              7153, -9500, 9500, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BOS)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BOS,
                                                                              10570, -13500, 13500, ystep, outer_gap, outer_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BOL)] = RegionDescriptor(1, DetRegIdx::Barrel, ChIdx::BOL,
                                                                             9500, -13500, 13500, ystep, outer_gap, outer_step);
        m_regionDescriptions[static_cast<int>(ChIdx::BEE)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::BEE, 
                                                                              4415, 7500, 13000, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EIS)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EIS, 
                                                                              7270, 1000, 7000, ystep, inner_gap, inner_step);  // 7
        m_regionDescriptions[static_cast<int>(ChIdx::EIL)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EIL, 
                                                                              7675, 1000, 8000, ystep, inner_gap, inner_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EES)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EES, 
                                                                              10800, 4000, 10000, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EEL)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EEL, 
                                                                              11330, 4000, 10000, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EMS)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EMS, 
                                                                              13872, 1500, 13000, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EML)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EML, 
                                                                              14310, 1500, 13000, ystep, middle_gap, middle_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EOS)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EOS, 
                                                                              21841, 2000, 13500, ystep, outer_gap, outer_step);
        m_regionDescriptions[static_cast<int>(ChIdx::EOL)] = RegionDescriptor(1, DetRegIdx::EndcapA, ChIdx::EOL, 
                                                                              21421, 2000, 13500, ystep, outer_gap, outer_step);
    }

}  // namespace MuonHough
