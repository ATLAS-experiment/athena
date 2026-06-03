/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

bool TGCModuleId::operator==(const TGCModuleId& moduleId) const {
    if ((this->getModuleIdType() == moduleId.getModuleIdType()) &&
        (this->getSideType() == moduleId.getSideType()) &&
        (this->getRegionType() == moduleId.getRegionType()) &&
        (this->getSignalType() == moduleId.getSignalType()) &&
        (this->getModuleType() == moduleId.getModuleType()) &&
        (this->getSector() == moduleId.getSector()) &&
        (this->getOctant() == moduleId.getOctant()) &&
        (this->getId() == moduleId.getId())) {
        return true;
    }
    return false;
}

void TGCModuleId::setSector(int v_sector) {
    m_sector = v_sector;
    if (m_region == RegionType::Endcap) {
        if (m_module == ModuleType::WI || m_module == ModuleType::SI) {
            m_octant = m_sector / (NUM_INNER_SECTOR / NUM_OCTANT);
            m_sectorRO = m_sector / (NUM_INNER_SECTOR / N_RODS);
        } else {
            m_octant = m_sector / (NUM_ENDCAP_SECTOR / NUM_OCTANT);
            m_sectorRO = m_sector / (NUM_ENDCAP_SECTOR / N_RODS);
        }
    } else if (m_region == RegionType::Forward) {
        m_octant = m_sector / (NUM_FORWARD_SECTOR / NUM_OCTANT);
        m_sectorRO = m_sector / (NUM_FORWARD_SECTOR / N_RODS);
    }
}

}  // namespace MuonTGC_Cabling
