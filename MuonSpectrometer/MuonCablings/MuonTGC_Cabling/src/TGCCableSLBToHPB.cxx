/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCableSLBToHPB.h"

#include "MuonTGC_Cabling/TGCChannelHPBIn.h"
#include "MuonTGC_Cabling/TGCChannelSLBOut.h"
#include "MuonTGC_Cabling/TGCDatabasePPToSL.h"
#include "MuonTGC_Cabling/TGCModuleHPB.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

TGCCableSLBToHPB::TGCCableSLBToHPB(const std::string& filename)
    : TGCCable(TGCCable::SLBToHPB), m_database{{{nullptr}}} {
    m_database[TGCId::Endcap][TGCId::WT] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB EWT");
    m_database[TGCId::Endcap][TGCId::WD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB EWD");
    m_database[TGCId::Endcap][TGCId::ST] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB EST");
    m_database[TGCId::Endcap][TGCId::SD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB ESD");
    m_database[TGCId::Forward][TGCId::WT] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB FWT");
    m_database[TGCId::Forward][TGCId::WD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB FWD");
    m_database[TGCId::Forward][TGCId::ST] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB FST");
    m_database[TGCId::Forward][TGCId::SD] =
        std::make_unique<TGCDatabasePPToSL>(filename, "SB FSD");
}

TGCCableSLBToHPB::~TGCCableSLBToHPB() = default;

std::unique_ptr<TGCChannelId> TGCCableSLBToHPB::getChannel(
    const TGCChannelId& channelId, bool orChannel) const {

    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::SLBOut) {
        return getChannelOut(channelId, orChannel);
    }
    if (channelId.getChannelIdType() == TGCChannelId::ChannelIdType::HPBIn) {
        return getChannelIn(channelId, orChannel);
    }

    return nullptr;
}

TGCModuleMap TGCCableSLBToHPB::getModule(const TGCModuleId& moduleId) const {

    if (moduleId.getModuleIdType() == TGCModuleId::SLB) {
        return getModuleOut(moduleId);
    }
    if (moduleId.getModuleIdType() == TGCModuleId::HPB) {
        return getModuleIn(moduleId);
    }

    return TGCModuleMap{};
}

std::unique_ptr<TGCChannelId> TGCCableSLBToHPB::getChannelIn(
    const TGCChannelId& hpbin, bool orChannel) const {
    if (orChannel || hpbin.isValid() == false) {
        return nullptr;
    }

    // HPB module
    std::unique_ptr<TGCModuleId> hpb{hpbin.getModule()};
    if (!hpb) {
        return nullptr;
    }

    // HPB -> SLB module connection
    TGCModuleMap mapId = getModule(*hpb);

    if (!mapId.size()) {
        return nullptr;
    }

    int numOfBlock = TGCChannelSLBOut::getNumberOfBlock(TGCId::WD);  // SD
    if (numOfBlock == 0) {
        return nullptr;
    }

    int chInBlock = TGCChannelSLBOut::getChannelInBlock(TGCId::WD);  // SD
    int slbInBlock = TGCChannelHPBIn::getSlbInBlock();

    // SLB module

    int blockInHPB = hpbin.getBlock();
    int nSlb = hpbin.getChannel() / (numOfBlock * 2);  // half block
    int port = blockInHPB * slbInBlock + nSlb;
    auto slb = mapId.popModule(port);

    if (!slb) {
        return nullptr;
    }

    // HPB ->SLB channel connection
    int nInSlb = hpbin.getChannel() / (numOfBlock * 2);  // half block
    int blockInSlb = nInSlb / 2;
    int nInBlock = nInSlb % 2;
    int channel = (chInBlock / 2) * nInBlock;

    return std::make_unique<TGCChannelSLBOut>(
        slb->getSideType(), slb->getModuleType(), slb->getRegionType(),
        slb->getSector(), slb->getId(), blockInSlb, channel);
}

std::unique_ptr<TGCChannelId> TGCCableSLBToHPB::getChannelInforHPB(
    const TGCChannelId& hpbin, TGCId::ModuleType moduleType,
    bool orChannel) const {
    if (orChannel || hpbin.isValid() == false) {
        return nullptr;
    }
    // HPB module
    std::unique_ptr<TGCModuleId> hpb{
        hpbin.getModule()};  // This function is defined in TGCChannelHPBIn
                             // set current SideType,SignalType...
                             // ChannelID(HPBIn) is set in HPBIn constructor
    if (!hpb) {
        return nullptr;
    }

    // HPB -> SLB module connection
    TGCModuleMap mapId = getModuleInforHPB(*hpb, moduleType);
    if (!mapId.size()) {
        return nullptr;
    }

    // need to check
    int numOfBlock = TGCChannelSLBOut::getNumberOfBlock(moduleType);
    if (numOfBlock == 0) {
        return nullptr;
    }

    int chInBlock = TGCChannelSLBOut::getChannelInBlock(moduleType);
    int slbInBlock = 0;

    // SLB module
    std::unique_ptr<TGCModuleId> slb{};
    int blockInHPB = 0;
    int nSlb = 0;  // half block
    int port = 0;  // input position for HPT

    // HPB ->SLB channel connection
    int blockInSlb = 0;
    int posInHpb = 0;
    int channel = 0;
    switch (moduleType) {
        case TGCId::WD:
        case TGCId::SD:
            slbInBlock = 3;
            blockInHPB = hpbin.getBlock();
            nSlb = hpbin.getChannel() / (numOfBlock * 2);
            port = blockInHPB * slbInBlock + nSlb;
            slb = mapId.popModule(port);
            blockInSlb = (hpbin.getChannel() - nSlb * (numOfBlock * 2)) / 2;
            posInHpb = (hpbin.getChannel() - (numOfBlock * 2) * nSlb) % 2;
            channel = (chInBlock / 2) * posInHpb;
            break;

        case TGCId::WT:
            slbInBlock = 2;
            blockInHPB = hpbin.getBlock();
            nSlb = hpbin.getChannel() / (numOfBlock * 2);
            port = blockInHPB * slbInBlock + nSlb;
            slb = mapId.popModule(-port);

            blockInSlb = (hpbin.getChannel() - nSlb * (numOfBlock * 2)) / 2;
            posInHpb = (hpbin.getChannel() - (numOfBlock * 2) * nSlb) % 2;
            channel = (chInBlock / 2) * posInHpb;
            break;

        case TGCId::ST:
            blockInHPB = hpbin.getBlock();
            if (blockInHPB == 0 && hpbin.getChannel() < numOfBlock) {
                port = 1;
            } else {
                port = 2;
            }
            slb = mapId.popModule(-port);

            blockInSlb = (hpbin.getChannel()) % numOfBlock;
            if (blockInHPB == 1 && hpbin.getChannel() >= numOfBlock) {
                blockInSlb = blockInSlb + numOfBlock / 2;
            }
            channel = 0;
            break;

        default:
            break;
    }

    if (!slb) {
        return nullptr;
    }

    const int offset = 6;  // offset to get center of block
    channel += offset;

    int SLBID = slb->getId();
    if ((chInBlock * blockInSlb + channel) > chInBlock * numOfBlock) {
        SLBID++;
        channel = channel % chInBlock;
        blockInSlb = 0;
    } else if (channel > chInBlock) {
        channel = channel % chInBlock;
        blockInSlb++;
    }

    return std::make_unique<TGCChannelSLBOut>(
        slb->getSideType(), moduleType, slb->getRegionType(), slb->getSector(),
        SLBID, blockInSlb, channel);
}

std::unique_ptr<TGCChannelId> TGCCableSLBToHPB::getChannelOut(
    const TGCChannelId& slbout, bool orChannel) const {
    if (orChannel || slbout.isValid() == false ||
        slbout.getMultipletType() == TGCId::Triplet) {
        return nullptr;
    }

    // SLB module
    std::unique_ptr<TGCModuleId> slb{slbout.getModule()};
    if (!slb) {
        return nullptr;
    }

    // SLB -> HPB module connection
    TGCModuleMap mapId = getModule(*slb);

    if (mapId.empty()) {
        return nullptr;
    }

    // HPB module
    int port = mapId.begin()->first;
    std::unique_ptr<TGCModuleId> hpb = mapId.popModule(port);
    if (!hpb) {
        return nullptr;
    }

    int chInBlock = TGCChannelSLBOut::getChannelInBlock(slbout.getModuleType());
    if (chInBlock <= 1) {
        return nullptr;
    }
    int slbInBlock = TGCChannelHPBIn::getSlbInBlock();

    // SLB ->HPB channel connection
    int block = port / slbInBlock;
    int hitId = (port % slbInBlock) * 2 + slbout.getBlock();
    int pos = slbout.getChannel() / (chInBlock / 2);
    int channel = hitId * 2 + pos;

    return std::make_unique<TGCChannelHPBIn>(
        hpb->getSideType(), hpb->getSignalType(), hpb->getRegionType(),
        hpb->getSector(), hpb->getId(), block, channel);
}

TGCModuleMap TGCCableSLBToHPB::getModuleIn(const TGCModuleId& hpbin) const {
    if (hpbin.isValid() == false) {
        return TGCModuleMap{};
    }
    const int hpbId = hpbin.getId();

    TGCId::ModuleType doublet = TGCId::NoModuleType;
    TGCId::ModuleType triplet = TGCId::NoModuleType;
    if (hpbin.getSignalType() == TGCId::Wire) {
        doublet = TGCId::WD;
        triplet = TGCId::WT;
    }
    if (hpbin.getSignalType() == TGCId::Strip) {
        doublet = TGCId::SD;
        triplet = TGCId::ST;
    }

    if (doublet == TGCId::NoModuleType || triplet == TGCId::NoModuleType) {
        return TGCModuleMap{};
    }

    TGCDatabase* doubletP = m_database[hpbin.getRegionType()][doublet].get();
    TGCDatabase* tripletP = m_database[hpbin.getRegionType()][triplet].get();

    TGCModuleMap mapId{};
    const int doubletMaxEntry = doubletP->getMaxEntry();
    for (int i = 0; i < doubletMaxEntry; i++) {
        if (doubletP->getEntry(i, 1) == hpbId) {
            int id = doubletP->getEntry(i, 0);
            int block = doubletP->getEntry(i, 2);
            auto slb = std::make_unique<TGCModuleSLB>(
                hpbin.getSideType(), doublet, hpbin.getRegionType(),
                hpbin.getSector(), id);

            mapId.insert(block, std::move(slb));
        }
    }
    const int tripletMaxEntry = tripletP->getMaxEntry();
    for (int i = 0; i < tripletMaxEntry; i++) {
        if (tripletP->getEntry(i, 1) == hpbId) {
            int id = tripletP->getEntry(i, 0);
            int block = -tripletP->getEntry(i, 2);
            auto slb = std::make_unique<TGCModuleSLB>(
                hpbin.getSideType(), triplet, hpbin.getRegionType(),
                hpbin.getSector(), id);

            mapId.insert(block, std::move(slb));
        }
    }

    return mapId;
}

TGCModuleMap TGCCableSLBToHPB::getModuleInforHPB(
    const TGCModuleId& hpbin, TGCId::ModuleType moduleType) const {
    if (hpbin.isValid() == false) {
        return TGCModuleMap{};
    }

    const int hpbId = hpbin.getId();

    TGCDatabase* databaseP =
        m_database[hpbin.getRegionType()][moduleType].get();

    TGCModuleMap mapId{};
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (databaseP->getEntry(i, 1) == hpbId) {
            int id = databaseP->getEntry(i, 0);
            int block = -100;
            switch (moduleType) {
                case TGCId::WD:
                case TGCId::SD:
                    block = databaseP->getEntry(i, 2);
                    break;

                case TGCId::WT:
                case TGCId::ST:
                    block = -1 * databaseP->getEntry(i, 2);  // check - sign
                    break;

                default:
                    break;
            }
            auto slb = std::make_unique<TGCModuleSLB>(
                hpbin.getSideType(), moduleType, hpbin.getRegionType(),
                hpbin.getSector(), id);

            mapId.insert(block, std::move(slb));
        }
    }

    return mapId;
}

TGCModuleMap TGCCableSLBToHPB::getModuleOut(const TGCModuleId& slb) const {
    if (slb.isValid() == false) {
        return TGCModuleMap{};
    }

    const int slbId = slb.getId();

    TGCDatabase* databaseP =
        m_database[slb.getRegionType()][slb.getModuleType()].get();

    TGCModuleMap mapId{};
    const int MaxEntry = databaseP->getMaxEntry();
    for (int i = 0; i < MaxEntry; i++) {
        if (databaseP->getEntry(i, 0) == slbId) {
            int id = databaseP->getEntry(i, 1);
            int block = databaseP->getEntry(i, 2);
            if (slb.getMultipletType() == TGCId::Triplet) {
                block *= -1;
            }

            auto hpb = std::make_unique<TGCModuleHPB>(
                slb.getSideType(), slb.getSignalType(), slb.getRegionType(),
                slb.getSector(), id);

            mapId.insert(block, std::move(hpb));
            break;
        }
    }

    return mapId;
}

}  // namespace MuonTGC_Cabling
