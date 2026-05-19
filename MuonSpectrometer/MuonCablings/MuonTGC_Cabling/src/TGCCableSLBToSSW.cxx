/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableSLBToSSW.h"

#include "MuonTGC_Cabling/TGCDatabaseSLBToROD.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"
#include "MuonTGC_Cabling/TGCModuleSSW.h"

namespace MuonTGC_Cabling {

TGCCableSLBToSSW::TGCCableSLBToSSW(const std::string& filename)
    : TGCCable(TGCCable::SLBToSSW), m_database{{{nullptr}}} {
    m_database.at(TGCId::Endcap).at(TGCId::WT) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB EWT");
    m_database.at(TGCId::Endcap).at(TGCId::WD) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB EWD");
    m_database.at(TGCId::Endcap).at(TGCId::ST) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB EST");
    m_database.at(TGCId::Endcap).at(TGCId::SD) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB ESD");
    m_database.at(TGCId::Endcap).at(TGCId::WI) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB EWI");
    m_database.at(TGCId::Endcap).at(TGCId::SI) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB ESI");
    m_database.at(TGCId::Endcap).at(SL) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB ESL");

    m_database.at(TGCId::Forward).at(TGCId::WT) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FWT");
    m_database.at(TGCId::Forward).at(TGCId::WD) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FWD");
    m_database.at(TGCId::Forward).at(TGCId::ST) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FST");
    m_database.at(TGCId::Forward).at(TGCId::SD) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FSD");
    m_database.at(TGCId::Forward).at(TGCId::WI) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FWI");
    m_database.at(TGCId::Forward).at(TGCId::SI) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FSI");
    m_database.at(TGCId::Forward).at(SL) =
        std::make_unique<TGCDatabaseSLBToROD>(filename, "SLB FSL");
}

TGCCableSLBToSSW::~TGCCableSLBToSSW() = default;

TGCModuleMap TGCCableSLBToSSW::getModule(const TGCModuleId& moduleId) const {

    if (moduleId.getModuleIdType() == TGCModuleId::SLB) {
        return getModuleOut(moduleId);
    }
    if (moduleId.getModuleIdType() == TGCModuleId::SSW) {
        return getModuleIn(moduleId);
    }

    return TGCModuleMap{};
}

TGCModuleMap TGCCableSLBToSSW::getModuleIn(const TGCModuleId& ssw) const {
    if (ssw.isValid() == false) {
        return TGCModuleMap{};
    }

    const int sswId = ssw.getId();

    TGCDatabase* databaseP[TGCId::MaxRegionType * MaxModuleType];
    TGCId::ModuleType module[TGCId::MaxRegionType * MaxModuleType];
    TGCId::RegionType region[TGCId::MaxRegionType * MaxModuleType];
    for (int i = 0; i < TGCId::MaxRegionType; i++) {
        for (int j = 0; j < MaxModuleType; j++) {
            databaseP[i * MaxModuleType + j] = m_database.at(i).at(j).get();
            region[i * MaxModuleType + j] = static_cast<TGCId::RegionType>(i);
            module[i * MaxModuleType + j] = static_cast<TGCId::ModuleType>(j);
        }
    }

    TGCModuleMap mapId{};
    for (int type = 0; type < TGCId::MaxRegionType * MaxModuleType; type++) {
        const int MaxEntry = databaseP[type]->getMaxEntry();
        for (int i = 0; i < MaxEntry; i++) {

            if (databaseP[type]->getEntry(i, 4) == sswId) {
                int sector = databaseP[type]->getEntry(i, 0);

                // Inner
                if (module[type] == TGCId::WI) {
                    // EI/FI
                    //  wire(TGCId::WI) and strip(TGCId::SI) of a chamber
                    //  use the same SLB chip

                    int sswSector = ssw.getReadoutSector();
                    // one ROD (rodId = 2,5,8,11) covers 6 Inner sectors
                    if (sswSector % 3 != 1) {
                        continue;
                    }
                    sswSector -= 1;

                    sector += sswSector * 2;
                    int id = databaseP[type]->getEntry(i, 1);
                    int block = databaseP[type]->getEntry(i, 5);
                    int sbLoc = databaseP[type]->getEntry(i, 2);
                    int slbAddr = databaseP[type]->getEntry(i, 3);
                    for (int ip = 0; ip < 3; ip++) {
                        int psector = sector + ip * 2;
                        int pblock = block + ip * 4;
                        int psbLoc = sbLoc + ip * 4;
                        int pslbAddr = slbAddr + ip * 2;
                        auto slb = std::make_unique<TGCModuleSLB>(
                            ssw.getSideType(), module[type], region[type],
                            psector, id, psbLoc, pslbAddr);

                        mapId.insert(pblock, std::move(slb));
                    }

                } else if (module[type] == TGCId::SI) {
                    // EI/FI
                    //  wire(TGCId::WI) and strip(TGCId::SI) of a chamber
                    //  use the same SLB chip

                    // do nothing

                } else {
                    if (region[type] == TGCId::Endcap) {
                        sector += ssw.getReadoutSector() *
                                  (TGCId::NUM_ENDCAP_SECTOR / TGCId::N_RODS);
                    } else {
                        sector += ssw.getReadoutSector() *
                                  (TGCId::NUM_FORWARD_SECTOR / TGCId::N_RODS);
                    }
                    int id = databaseP[type]->getEntry(i, 1);
                    int sbLoc = databaseP[type]->getEntry(i, 2);
                    int slbAddr = databaseP[type]->getEntry(i, 3);
                    int block = databaseP[type]->getEntry(i, 5);
                    auto slb = std::make_unique<TGCModuleSLB>(
                        ssw.getSideType(), module[type], region[type], sector,
                        id, sbLoc, slbAddr);

                    mapId.insert(block, std::move(slb));
                }
            }
        }
    }

    return mapId;
}

TGCModuleMap TGCCableSLBToSSW::getModuleOut(const TGCModuleId& slb) const {
    if (slb.isValid() == false) {
        return TGCModuleMap{};
    }

    const int slbId = slb.getId();
    const int sector = slb.getSectorInReadout();
    int readoutSector = slb.getReadoutSector();

    TGCModuleMap mapId{};

    TGCDatabase* databaseP =
        m_database.at(slb.getRegionType()).at(slb.getModuleType()).get();
    const int MaxEntry = databaseP->getMaxEntry();
    if (slb.getModuleType() == TGCId::WI || slb.getModuleType() == TGCId::SI) {
        // inner
        for (int i = 0; i < MaxEntry; i++) {
            if (databaseP->getEntry(i, 0) == sector &&
                databaseP->getEntry(i, 1) == slbId) {

                int sswSectorRO = readoutSector - (readoutSector % 3) + 1;
                int id = databaseP->getEntry(i, 4);
                int block = databaseP->getEntry(i, 5) + 4 * (readoutSector % 3);

                auto ssw = std::make_unique<TGCModuleSSW>(slb.getSideType(),
                                                          sswSectorRO, id);

                mapId.insert(block, std::move(ssw));
                break;
            }
        }

    } else {
        for (int i = 0; i < MaxEntry; i++) {
            if (databaseP->getEntry(i, 0) == sector &&
                databaseP->getEntry(i, 1) == slbId) {

                int id = databaseP->getEntry(i, 4);
                int block = databaseP->getEntry(i, 5);
                auto ssw = std::make_unique<TGCModuleSSW>(slb.getSideType(),
                                                          readoutSector, id);

                mapId.insert(block, std::move(ssw));
                break;
            }
        }
    }
    return mapId;
}

}  // namespace MuonTGC_Cabling
