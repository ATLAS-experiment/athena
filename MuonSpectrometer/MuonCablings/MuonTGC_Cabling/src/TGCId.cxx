/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
    if (m_module == WI) {
        setSignalType(Wire);
        setMultipletType(Inner);
    }
    if (m_module == SI) {
        setSignalType(Strip);
        setMultipletType(Inner);
    }
    if (m_module == WD) {
        setSignalType(Wire);
        setMultipletType(Doublet);
    }
    if (m_module == SD) {
        setSignalType(Strip);
        setMultipletType(Doublet);
    }
    if (m_module == WT) {
        setSignalType(Wire);
        setMultipletType(Triplet);
    }
    if (m_module == ST) {
        setSignalType(Strip);
        setMultipletType(Triplet);
    }
}

void TGCId::setSignalType(SignalType v_signal) {
    m_signal = v_signal;
    if (m_multiplet == Inner && m_signal == Wire) {
        m_module = WI;
    }
    if (m_multiplet == Inner && m_signal == Strip) {
        m_module = SI;
    }
    if (m_multiplet == Doublet && m_signal == Wire) {
        m_module = WD;
    }
    if (m_multiplet == Doublet && m_signal == Strip) {
        m_module = SD;
    }
    if (m_multiplet == Triplet && m_signal == Wire) {
        m_module = WT;
    }
    if (m_multiplet == Triplet && m_signal == Strip) {
        m_module = ST;
    }
}

void TGCId::setMultipletType(MultipletType v_multiplet) {
    this->m_multiplet = v_multiplet;
    if (m_multiplet == Inner && m_signal == Wire) {
        m_module = WI;
    }
    if (m_multiplet == Inner && m_signal == Strip) {
        m_module = SI;
    }
    if (m_multiplet == Doublet && m_signal == Wire) {
        m_module = WD;
    }
    if (m_multiplet == Doublet && m_signal == Strip) {
        m_module = SD;
    }
    if (m_multiplet == Triplet && m_signal == Wire) {
        m_module = WT;
    }
    if (m_multiplet == Triplet && m_signal == Strip) {
        m_module = ST;
    }
}

void TGCId::setStation(int v_station) {
    m_station = v_station;
    if (m_station == 0) {
        setMultipletType(Triplet);
    }
    if (m_station == 1) {
        setMultipletType(Doublet);
    }
    if (m_station == 2) {
        setMultipletType(Doublet);
    }
    if (m_station == 3) {
        setMultipletType(Inner);
    }
}

void TGCId::setSector(int v_sector) {
    m_sector = v_sector;
    if (m_region == Endcap) {
        if (m_multiplet == Inner) {
            m_octant = m_sector / (NUM_INNER_SECTOR / NUM_OCTANT);
        } else {
            m_octant = m_sector / (NUM_ENDCAP_SECTOR / NUM_OCTANT);
        }
    } else if (m_region == Forward) {
        m_octant = m_sector / (NUM_FORWARD_SECTOR / NUM_OCTANT);
    }
}

void TGCId::setOctant(int v_octant) {
    m_octant = v_octant;
}

int TGCId::getSectorModule(void) const {
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
        setRegionType(Endcap);
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_ENDCAP_SECTOR / NUM_OCTANT));

    } else if (regionId[sectorModule] == 1) {
        setRegionType(Forward);
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_FORWARD_SECTOR / NUM_OCTANT));
    } else {
        setMultipletType(Inner);
        if (regionId[sectorModule] == 2) {
            setRegionType(Endcap);
        }
        if (regionId[sectorModule] == 3) {
            setRegionType(Forward);
        }
        setSector(sectorId[sectorModule] +
                  m_octant * (NUM_INNER_SECTOR / NUM_OCTANT));
    }
}

}  // namespace MuonTGC_Cabling
