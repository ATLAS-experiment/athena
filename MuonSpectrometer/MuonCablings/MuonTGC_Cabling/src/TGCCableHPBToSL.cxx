/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableHPBToSL.h"

#include "MuonTGC_Cabling/TGCDatabasePPToSL.h"
#include "MuonTGC_Cabling/TGCModuleHPB.h"
#include "MuonTGC_Cabling/TGCModuleSL.h"

namespace MuonTGC_Cabling {

TGCCableHPBToSL::TGCCableHPBToSL(const std::string& filename)
    : TGCCable(TGCCable::HPBToSL), m_database{{{nullptr}}} {
    m_database.at(TGCId::Endcap).at(TGCId::Wire) =
        std::make_unique<TGCDatabasePPToSL>(filename, "HPB EW");
    m_database.at(TGCId::Endcap).at(TGCId::Strip) =
        std::make_unique<TGCDatabasePPToSL>(filename, "HPB ES");
    m_database.at(TGCId::Forward).at(TGCId::Wire) =
        std::make_unique<TGCDatabasePPToSL>(filename, "HPB FW");
    m_database.at(TGCId::Forward).at(TGCId::Strip) =
        std::make_unique<TGCDatabasePPToSL>(filename, "HPB FS");
}

TGCCableHPBToSL::~TGCCableHPBToSL() = default;

TGCModuleMap TGCCableHPBToSL::getModule(const TGCModuleId& moduleId) const {
    if (moduleId.getModuleIdType() == TGCModuleId::HPB) {
        return getModuleOut(moduleId);
    }
    if (moduleId.getModuleIdType() == TGCModuleId::SL) {
        return getModuleIn(moduleId);
    }
    return TGCModuleMap{};
}

TGCModuleMap TGCCableHPBToSL::getModuleIn(const TGCModuleId& sl) const {
    if (sl.isValid() == false) {
        return TGCModuleMap{};
    }

    TGCDatabase* wireP = m_database.at(sl.getRegionType()).at(TGCId::Wire).get();
    TGCDatabase* stripP = m_database.at(sl.getRegionType()).at(TGCId::Strip).get();

    TGCModuleMap mapId{};
    const int wireMaxEntry = wireP->getMaxEntry();
    for (int i = 0; i < wireMaxEntry; i++) {
        int id = wireP->getEntry(i, 0);
        int block = wireP->getEntry(i, 1);
        auto hpb = std::make_unique<TGCModuleHPB>(sl.getSideType(), TGCId::Wire,
                                                  sl.getRegionType(),
                                                  sl.getSector(), id);
        mapId.insert(block, std::move(hpb));
    }

    const int stripMaxEntry = stripP->getMaxEntry();
    for (int i = 0; i < stripMaxEntry; i++) {
        int id = stripP->getEntry(i, 0);
        int block = stripP->getEntry(i, 1);
        auto hpb = std::make_unique<TGCModuleHPB>(
            sl.getSideType(), TGCId::Strip, sl.getRegionType(), sl.getSector(),
            id);
        mapId.insert(block, std::move(hpb));
    }

    return mapId;
}

TGCModuleMap TGCCableHPBToSL::getModuleOut(const TGCModuleId& hpb) const {
    if (hpb.isValid() == false) {
        return TGCModuleMap{};
    }

    const int hpbId = hpb.getId();

    TGCDatabase* databaseP =
        m_database.at(hpb.getRegionType()).at(hpb.getSignalType()).get();

    TGCModuleMap mapId{};
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (databaseP->getEntry(i, 0) == hpbId) {
            int block = databaseP->getEntry(i, 1);
            auto sl = std::make_unique<TGCModuleSL>(
                hpb.getSideType(), hpb.getRegionType(), hpb.getSector());

            mapId.insert(block, std::move(sl));
            break;
        }
    }

    return mapId;
}

}  // namespace MuonTGC_Cabling
