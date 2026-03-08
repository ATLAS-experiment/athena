/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableSSWToROD.h"

#include "MuonTGC_Cabling/TGCDatabaseSLBToROD.h"
#include "MuonTGC_Cabling/TGCModuleROD.h"
#include "MuonTGC_Cabling/TGCModuleSSW.h"

namespace MuonTGC_Cabling {

TGCCableSSWToROD::TGCCableSSWToROD(const std::string& filename)
    : TGCCable(TGCCable::SSWToROD),
      m_database(std::make_unique<TGCDatabaseSLBToROD>(filename, "SSW ALL")) {}

TGCCableSSWToROD::TGCCableSSWToROD(const TGCCableSSWToROD& right)
    : TGCCable(TGCCable::SSWToROD) {
    if (auto mypointer =
            dynamic_cast<TGCDatabaseSLBToROD*>(right.m_database.get())) {
        m_database = std::make_unique<TGCDatabaseSLBToROD>(*mypointer);
    } else {
        m_database.reset();
    }
}

TGCCableSSWToROD& TGCCableSSWToROD::operator=(const TGCCableSSWToROD& right) {
    if (this != &right) {
        if (auto mypointer =
                dynamic_cast<TGCDatabaseSLBToROD*>(right.m_database.get())) {
            m_database = std::make_unique<TGCDatabaseSLBToROD>(*mypointer);
        } else {
            m_database.reset();
        }
    }
    return *this;
}

TGCCableSSWToROD::~TGCCableSSWToROD() = default;

TGCModuleMap TGCCableSSWToROD::getModule(const TGCModuleId& moduleId) const {

    if (moduleId.getModuleIdType() == TGCModuleId::SSW) {
        return getModuleOut(moduleId);
    }
    if (moduleId.getModuleIdType() == TGCModuleId::ROD) {
        return getModuleIn(moduleId);
    }

    return TGCModuleMap{};
}

TGCModuleMap TGCCableSSWToROD::getModuleIn(const TGCModuleId& rod) const {
    if (!rod.isValid()) {
        return TGCModuleMap{};
    }

    const TGCId::SideType rodSideType = rod.getSideType();
    const int rodReadoutSector = rod.getReadoutSector();

    TGCModuleMap mapId{};
    const int MaxEntry = m_database->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        int id = m_database->getEntry(i, 0);
        int block = m_database->getEntry(i, 1);
        auto ssw =
            std::make_unique<TGCModuleSSW>(rodSideType, rodReadoutSector, id);
        mapId.insert(block, std::move(ssw));
    }
    return mapId;
}

TGCModuleMap TGCCableSSWToROD::getModuleOut(const TGCModuleId& ssw) const {
    if (!ssw.isValid()) {
        return TGCModuleMap{};
    }

    const int sswId = ssw.getId();

    TGCModuleMap mapId{};
    const int MaxEntry = m_database->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (m_database->getEntry(i, 0) == sswId) {
            int block = m_database->getEntry(i, 1);
            auto rod = std::make_unique<TGCModuleROD>(ssw.getSideType(),
                                                      ssw.getReadoutSector());

            mapId.insert(block, std::move(rod));
            break;
        }
    }
    return mapId;
}

}  // namespace MuonTGC_Cabling
