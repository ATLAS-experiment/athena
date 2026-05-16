/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCablePPToSLB.h"

#include "MuonTGC_Cabling/TGCChannelPPOut.h"
#include "MuonTGC_Cabling/TGCChannelSLBIn.h"
#include "MuonTGC_Cabling/TGCDatabasePPToSL.h"
#include "MuonTGC_Cabling/TGCModulePP.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

// Constructor & Destructor
TGCCablePPToSLB::TGCCablePPToSLB(const std::string& filename)
    : TGCCable(TGCCable::PPToSLB), m_database{{{nullptr}}} {
    m_database[TGCId::Endcap][TGCId::WT] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP EWT");
    m_database[TGCId::Endcap][TGCId::WD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP EWD");
    m_database[TGCId::Endcap][TGCId::ST] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP EST");
    m_database[TGCId::Endcap][TGCId::SD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP ESD");
    m_database[TGCId::Endcap][TGCId::WI] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP EWI");
    m_database[TGCId::Endcap][TGCId::SI] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP ESI");
    m_database[TGCId::Forward][TGCId::WT] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FWT");
    m_database[TGCId::Forward][TGCId::WD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FWD");
    m_database[TGCId::Forward][TGCId::ST] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FST");
    m_database[TGCId::Forward][TGCId::SD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FSD");
    m_database[TGCId::Forward][TGCId::WI] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FWI");
    m_database[TGCId::Forward][TGCId::SI] =
        std::make_unique<TGCDatabasePPToSL>(filename, "PP FSI");
}

TGCCablePPToSLB::~TGCCablePPToSLB() = default;

std::unique_ptr<TGCChannelId> TGCCablePPToSLB::getChannel(
    const TGCChannelId& channelId, bool orChannel) const {
    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::PPOut) {
        return getChannelOut(channelId, orChannel);
    }
    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::SLBIn) {
        return getChannelIn(channelId, orChannel);
    }

    return nullptr;
}

TGCModuleMap TGCCablePPToSLB::getModule(const TGCModuleId& moduleId) const {
    if (moduleId.getModuleIdType() == TGCModuleId::PP) {
        return getModuleOut(moduleId);
    }
    if (moduleId.getModuleIdType() == TGCModuleId::SLB) {
        return getModuleIn(moduleId);
    }
    return TGCModuleMap{};
}

std::unique_ptr<TGCChannelId> TGCCablePPToSLB::getChannelIn(
    const TGCChannelId& slbin, bool orChannel) const {
    if (orChannel || slbin.isValid() == false) {
        return nullptr;
    }

    // SLB channel
    const TGCChannelSLBIn* slbIn = nullptr;
    if (slbin.getChannelIdType() == TGCChannelId::ChannelIdType::SLBIn) {
        slbIn = dynamic_cast<const TGCChannelSLBIn*>(&slbin);
    }
    if (!slbIn) {
        return nullptr;
    }

    // SLB module
    std::unique_ptr<TGCModuleId> slb = slbIn->getModule();
    if (!slb) {
        return nullptr;
    }

    // SLB -> PP module connection
    TGCModuleMap mapId = getModule(*slb);
    if (!mapId.size()) {
        return nullptr;
    }

    // PP module
    int port = mapId.begin()->first;
    auto pp = mapId.popModule(port);
    if (!pp) {
        return nullptr;
    }

    // SLB ->PP channel connection
    int block = -1;
    int channel = -1;
    TGCChannelSLBIn::CellType cellType = slbIn->getCellType();
    if (cellType == TGCChannelSLBIn::NoCellType) {
        return nullptr;
    }

    if (cellType == TGCChannelSLBIn::CellA) {
        block = 1 + 2 * port;
        int lengthOfSLB = TGCChannelSLBIn::getLengthOfSLB(
            slbIn->getModuleType(), TGCChannelSLBIn::CellB);
        channel = lengthOfSLB + slbIn->getChannelInSLB();
    }
    if (cellType == TGCChannelSLBIn::CellB) {
        block = 1 + 2 * port;
        channel = slbIn->getChannelInSLB();
    }
    if (cellType == TGCChannelSLBIn::CellC) {
        block = 0 + 2 * port;
        int lengthOfSLB = TGCChannelSLBIn::getLengthOfSLB(
            slbIn->getModuleType(), TGCChannelSLBIn::CellD);
        channel = lengthOfSLB + slbIn->getChannelInSLB();
    }
    if (cellType == TGCChannelSLBIn::CellD) {
        block = 0 + 2 * port;
        channel = slbIn->getChannelInSLB();
    }
    return std::make_unique<TGCChannelPPOut>(
        pp->getSideType(), pp->getModuleType(), pp->getRegionType(),
        pp->getSector(), pp->getId(), block, channel);
}

std::unique_ptr<TGCChannelId> TGCCablePPToSLB::getChannelOut(
    const TGCChannelId& ppout, bool orChannel) const {
    if (orChannel || ppout.isValid() == false) {
        return nullptr;
    }

    const int ppoutBlock = ppout.getBlock();

    // PP module
    std::unique_ptr<TGCModuleId> pp{ppout.getModule()};
    if (!pp) {
        return nullptr;
    }

    // PP -> SLB module connection
    TGCModuleMap mapId = getModule(*pp);
    if (!mapId.size()) {
        return nullptr;
    }

    // SLB module
    auto slb = mapId.popModule(ppoutBlock / 2);

    if (!slb) {
        return nullptr;
    }

    // PP ->SLB channel connection
    TGCChannelSLBIn::CellType cellType = TGCChannelSLBIn::NoCellType;
    int channelInSLB = -1;
    if (ppoutBlock % 2 == 0) {  // D,C
        int lengthOfSLB = TGCChannelSLBIn::getLengthOfSLB(
            slb->getModuleType(), TGCChannelSLBIn::CellD);
        if (ppout.getChannel() < lengthOfSLB) {
            cellType = TGCChannelSLBIn::CellD;
            channelInSLB = ppout.getChannel();
        } else {
            cellType = TGCChannelSLBIn::CellC;
            channelInSLB = ppout.getChannel() - lengthOfSLB;
        }
    } else {  // B,A
        int lengthOfSLB = TGCChannelSLBIn::getLengthOfSLB(
            slb->getModuleType(), TGCChannelSLBIn::CellB);
        if (ppout.getChannel() < lengthOfSLB) {
            cellType = TGCChannelSLBIn::CellB;
            channelInSLB = ppout.getChannel();
        } else {
            cellType = TGCChannelSLBIn::CellA;
            channelInSLB = ppout.getChannel() - lengthOfSLB;
        }
    }

    int channel = TGCChannelSLBIn::convertChannel(slb->getModuleType(),
                                                  cellType, channelInSLB);
    return std::make_unique<TGCChannelSLBIn>(
        slb->getSideType(), slb->getModuleType(), slb->getRegionType(),
        slb->getSector(), slb->getId(), channel);
}

TGCModuleMap TGCCablePPToSLB::getModuleIn(const TGCModuleId& slb) const {
    if (slb.isValid() == false) {
        return TGCModuleMap{};
    }

    const int slbId = slb.getId();

    TGCDatabase* databaseP =
        m_database[slb.getRegionIndex()][slb.getModuleIndex()].get();
    TGCModuleMap mapId{};
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (databaseP->getEntry(i, 1) == slbId) {
            int id = databaseP->getEntry(i, 0);
            auto pp = std::make_unique<TGCModulePP>(
                slb.getSideType(), slb.getModuleType(), slb.getRegionType(),
                slb.getSector(), id);

            mapId.insert(0, std::move(pp));
            break;
        }
        if (databaseP->getEntry(i, 2) == slbId) {
            int id = databaseP->getEntry(i, 0);
            auto pp = std::make_unique<TGCModulePP>(
                slb.getSideType(), slb.getModuleType(), slb.getRegionType(),
                slb.getSector(), id);

            mapId.insert(1, std::move(pp));
            break;
        }
    }

    return mapId;
}

TGCModuleMap TGCCablePPToSLB::getModuleOut(const TGCModuleId& pp) const {
    if (pp.isValid() == false) {
        return TGCModuleMap{};
    }

    const int ppId = pp.getId();

    TGCDatabase* databaseP =
        m_database[pp.getRegionIndex()][pp.getModuleIndex()].get();
    TGCModuleMap mapId{};
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (databaseP->getEntry(i, 0) == ppId) {
            int id = -1;

            id = databaseP->getEntry(i, 1);
            if (id != -1) {
                auto slb = std::make_unique<TGCModuleSLB>(
                    pp.getSideType(), pp.getModuleType(), pp.getRegionType(),
                    pp.getSector(), id);

                mapId.insert(0, std::move(slb));
            }

            id = databaseP->getEntry(i, 2);
            if (id != -1) {
                auto slb = std::make_unique<TGCModuleSLB>(
                    pp.getSideType(), pp.getModuleType(), pp.getRegionType(),
                    pp.getSector(), id);

                mapId.insert(1, std::move(slb));
            }
            break;
        }
    }

    return mapId;
}

}  // namespace MuonTGC_Cabling
