/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCId.h"

namespace MuonTGC_Cabling {

TGCId::TGCId(TGCId::IdType vtype) {
    m_idType = vtype;
}

int TGCId::getSectorInOctant() const {
    if (isInner()) {
        return getSector() % (NUM_INNER_SECTOR / NUM_OCTANT);
    }
    if (isEndcap()) {
        return getSector() % (NUM_ENDCAP_SECTOR / NUM_OCTANT);
    }
    if (isForward()) {
        return getSector() % (NUM_FORWARD_SECTOR / NUM_OCTANT);
    }
    return -1;
}

int TGCId::getSectorInReadout() const {
    if (isInner()) {
        return getSector() % (NUM_INNER_SECTOR / N_RODS);
    }
    if (isEndcap()) {
        return getSector() % (NUM_ENDCAP_SECTOR / N_RODS);
    }
    if (isForward()) {
        return getSector() % (NUM_FORWARD_SECTOR / N_RODS);
    }
    return -1;
}

void TGCId::setModuleType(ModuleType v_module) {
    m_module = v_module;
    if (m_module == ModuleType::WI) {
        setSignalType(SignalType::Wire);
    }
    if (m_module == ModuleType::SI) {
        setSignalType(SignalType::Strip);
    }
    if (m_module == ModuleType::WD) {
        setSignalType(SignalType::Wire);
    }
    if (m_module == ModuleType::SD) {
        setSignalType(SignalType::Strip);
    }
    if (m_module == ModuleType::WT) {
        setSignalType(SignalType::Wire);
    }
    if (m_module == ModuleType::ST) {
        setSignalType(SignalType::Strip);
    }
}

void TGCId::setSignalType(SignalType v_signal) {
    m_signal = v_signal;
    if (isInner() && m_signal == SignalType::Wire) {
        m_module = ModuleType::WI;
    } else if (isInner() && m_signal == SignalType::Strip) {
        m_module = ModuleType::SI;
    } else if (isDoublet() && m_signal == SignalType::Wire) {
        m_module = ModuleType::WD;
    } else if (isDoublet() && m_signal == SignalType::Strip) {
        m_module = ModuleType::SD;
    } else if (isTriplet() && m_signal == SignalType::Wire) {
        m_module = ModuleType::WT;
    } else if (isTriplet() && m_signal == SignalType::Strip) {
        m_module = ModuleType::ST;
    }
}

void TGCId::setStation(StationType v_station) {
    m_station = v_station;
}

void TGCId::setSector(int v_sector) {
    m_sector = v_sector;
    if (m_region == RegionType::Endcap) {
        if (isInner()) {
            m_octant = m_sector / (NUM_INNER_SECTOR / NUM_OCTANT);
        } else {
            m_octant = m_sector / (NUM_ENDCAP_SECTOR / NUM_OCTANT);
        }
    } else if (m_region == RegionType::Forward) {
        m_octant = m_sector / (NUM_FORWARD_SECTOR / NUM_OCTANT);
    }
}

void TGCId::setOctant(int v_octant) {
    m_octant = v_octant;
}

int TGCId::getSectorModule() const {
    if (m_sector == -1) {
        return -1;
    }

    static const int moduleEndcap[6] = {0, 1, 3, 4, 6, 7};
    static const int moduleForward[3] = {2, 5, 8};
    static const int moduleEI[3] = {9, 10, 11};
    static const int moduleFI[3] = {12, 13, 14};

    if (isEndcap()) {
        if (isInner()) {
            return moduleEI[getSectorInOctant()];
        }
        return moduleEndcap[getSectorInOctant()];
    }
    if (isForward()) {
        if (isInner()) {
            return moduleFI[getSectorInOctant()];
        }
        return moduleForward[getSectorInOctant()];
    }
    return -1;
}

// before this method, set m_octant.
void TGCId::setSectorModule(int sectorModule) {
    if (m_octant < 0) {
        return;
    }

    const int MaxModuleInOctant = 15;
    static const int regionId[MaxModuleInOctant] = {0, 0, 1, 0, 0, 1, 0, 0,
                                                    1, 2, 2, 2, 3, 3, 3};
    static const int sectorId[MaxModuleInOctant] = {0, 1, 0, 2, 3, 1, 4, 5,
                                                    2, 0, 1, 2, 0, 1, 2};

    if (sectorModule < 0 || sectorModule >= MaxModuleInOctant) {
        return;
    }

    if (regionId[sectorModule] == 0) {
        setRegionType(RegionType::Endcap);
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_ENDCAP_SECTOR / NUM_OCTANT));

    } else if (regionId[sectorModule] == 1) {
        setRegionType(RegionType::Forward);
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_FORWARD_SECTOR / NUM_OCTANT));
    } else {
        setStation(StationType::M4);
        if (regionId[sectorModule] == 2) {
            setRegionType(RegionType::Endcap);
        }
        if (regionId[sectorModule] == 3) {
            setRegionType(RegionType::Forward);
        }
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_INNER_SECTOR / NUM_OCTANT));
    }
}

}  // namespace MuonTGC_Cabling
