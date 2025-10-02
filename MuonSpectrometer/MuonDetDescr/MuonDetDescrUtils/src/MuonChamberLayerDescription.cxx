/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonDetDescrUtils/MuonChamberLayerDescription.h"

#include <iostream>

namespace Muon {

    MuonChamberLayerDescription::MuonChamberLayerDescription() { initDefaultRegions(); }

    MuonChamberLayerDescriptor MuonChamberLayerDescription::getDescriptor(int sector, DetRegIdx region, LayerIdx layer) const {
        bool isSmall = (sector % 2 == 0);
        using namespace MuonStationIndex;
        ChIndex chIndex = Muon::MuonStationIndex::toChamberIndex(region, layer, isSmall);
        
        
        if (chIndex == ChIndex::ChUnknown|| chIndex >= ChIndex::ChIndexMax) {
            MuonChamberLayerDescriptor descriptor;
            return descriptor;
        }

        MuonChamberLayerDescriptor descriptor = m_chamberLayerDescriptors[toInt(chIndex)];
        descriptor.sector = sector;
        // exceptions for a few barrel regions
        if (region == DetRegIdx::Barrel) {
            if ((sector == 10 || sector == 14) && layer == LayerIdx::Inner)
                descriptor.referencePosition = 5400.;
            else if ((sector == 11 || sector == 13) && layer == LayerIdx::Outer)
                descriptor.referencePosition = 10650.;
        } else if (region == DetRegIdx::EndcapC) {  // multiply reference position by -1 for C side
            descriptor.region = region;
            if (layer == LayerIdx::BarrelExtended) {
                descriptor.yMinRange *= -1;
                descriptor.yMaxRange *= -1;
                std::swap(descriptor.yMinRange, descriptor.yMaxRange);
            } else {
                descriptor.referencePosition *= -1;
            }
        }
        return descriptor;
    }

    void MuonChamberLayerDescription::initDefaultRegions() {
        using namespace MuonStationIndex;

        m_chamberLayerDescriptors.resize(toInt(ChIndex::CSS));
        m_chamberLayerDescriptors[toInt(ChIndex::BIS)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BIS, 4560, -7500, 7500, 30, 0.1, 3};
        m_chamberLayerDescriptors[toInt(ChIndex::BIL)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BIL, 4950, -7000, 7000, 30, 0.1, 3};
        m_chamberLayerDescriptors[toInt(ChIndex::BMS)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BMS, 8096, -9500, 9500, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::BML)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BML, 7153, -9500, 9500, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::BOS)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BOS, 10570, -13500, 13500, 30, 0.1, 7};
        m_chamberLayerDescriptors[toInt(ChIndex::BOL)] = MuonChamberLayerDescriptor{1, DetRegIdx::Barrel,  ChIndex::BOL, 9500, -13500, 13500, 30, 0.1, 7};
        m_chamberLayerDescriptors[toInt(ChIndex::BEE)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::BEE, 4415, 7500, 13000, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::EIS)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EIS, 7270, 1000, 7000, 30, .05, 3};
        m_chamberLayerDescriptors[toInt(ChIndex::EIL)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EIL, 7675, 1000, 8000, 30, .05, 3};
        m_chamberLayerDescriptors[toInt(ChIndex::EES)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EES, 10800, 4000, 10000, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::EEL)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EEL, 11330, 4000, 10000, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::EMS)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EMS, 13872, 1500, 13000, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::EML)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EML, 14310, 1500, 13000, 30, 0.1, 5};
        m_chamberLayerDescriptors[toInt(ChIndex::EOS)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EOS, 21841, 2000, 13500, 30, 0.1, 7};
        m_chamberLayerDescriptors[toInt(ChIndex::EOL)] = MuonChamberLayerDescriptor{1, DetRegIdx::EndcapA, ChIndex::EOL, 21421, 2000, 13500, 30, 0.1, 7};
    }

}  // namespace Muon
