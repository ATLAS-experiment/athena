/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TGCCabling.h"

#include "MuonTGC_Cabling/TGCCableASDToPP.h"
#include "MuonTGC_Cabling/TGCCableHPBToSL.h"
#include "MuonTGC_Cabling/TGCCableInASD.h"
#include "MuonTGC_Cabling/TGCCableInPP.h"
#include "MuonTGC_Cabling/TGCCableInSLB.h"
#include "MuonTGC_Cabling/TGCCablePPToSLB.h"
#include "MuonTGC_Cabling/TGCCableSLBToHPB.h"
#include "MuonTGC_Cabling/TGCCableSLBToSSW.h"
#include "MuonTGC_Cabling/TGCCableSSWToROD.h"
#include "MuonTGC_Cabling/TGCChannelASDOut.h"
#include "MuonTGC_Cabling/TGCChannelHPBIn.h"
#include "MuonTGC_Cabling/TGCChannelSLBIn.h"
#include "MuonTGC_Cabling/TGCChannelSLBOut.h"
#include "MuonTGC_Cabling/TGCModuleROD.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"

namespace MuonTGC_Cabling {

// Constructor & Destructor
TGCCabling::TGCCabling(const Config& cfg)
    : m_cableInASD{std::make_unique<TGCCableInASD>(cfg.fileNameASDtoPP)},
      m_cableASDToPP{std::make_unique<TGCCableASDToPP>(
          cfg.fileNameASDtoPP, cfg.fileNameASDtoPPdiff)},
      m_cableInPP{std::make_unique<TGCCableInPP>(cfg.fileNameInPP)},
      m_cablePPToSLB{std::make_unique<TGCCablePPToSLB>(cfg.fileNamePPtoSL)},
      m_cableInSLB{std::make_unique<TGCCableInSLB>()},
      m_cableSLBToHPB{std::make_unique<TGCCableSLBToHPB>(cfg.fileNamePPtoSL)},
      m_cableHPBToSL{std::make_unique<TGCCableHPBToSL>(cfg.fileNamePPtoSL)},
      m_cableSLBToSSW{std::make_unique<TGCCableSLBToSSW>(cfg.fileNameSLBtoROD)},
      m_cableSSWToROD{
          std::make_unique<TGCCableSSWToROD>(cfg.fileNameSLBtoROD)} {}

TGCCabling::~TGCCabling() = default;
// slbIn --> AsdOut
std::unique_ptr<TGCChannelId> TGCCabling::getASDOutChannel(
    const TGCChannelId& in) const {
    TGCChannelSLBIn slb_in(in.getSideType(), in.getModuleType(),
                           in.getRegionType(), in.getSector(), in.getId(),
                           in.getChannel());
    return getChannel(slb_in, TGCChannelId::ChannelIdType::ASDOut);
}

// readout ID -> SLB Module
const TGCModuleId* TGCCabling::getSLBFromReadout(TGCId::SideType side,
                                                 int rodId, int sswId,
                                                 int sbLoc) const {
    std::scoped_lock lock(m_mutex);

    int indexFromReadoutWithoutChannel =
        getIndexFromReadoutWithoutChannel(side, rodId, sswId, sbLoc);
    const auto it = m_slbModuleIdMap.find(indexFromReadoutWithoutChannel);
    if (it != m_slbModuleIdMap.end()) {
        // Already seen this ReadoutID without channel.
        // Stored pointer is returned.
        return it->second.get();
    }

    // ROD Module
    int readoutSector = rodId - 1;  // rodID = 1..12
    TGCModuleROD rod(side, readoutSector);

    // SSW Module
    TGCModuleMap sswMap = getModule(rod, TGCModuleId::SSW);
    if (sswMap.empty()) {
        m_slbModuleIdMap.insert(
            std::make_pair(indexFromReadoutWithoutChannel, nullptr));
        return nullptr;
    }

    std::unique_ptr<TGCModuleId> ssw = sswMap.popModule(sswId);
    if (!ssw) {
        m_slbModuleIdMap.insert(
            std::make_pair(indexFromReadoutWithoutChannel, nullptr));
        return nullptr;  // Do not need to delete ssw here.
                         // We can delete ssw but nothing will be done.
    }

    // SLB Module
    TGCModuleMap slbMap = getModule(*ssw, TGCModuleId::SLB);

    if (slbMap.empty()) {
        m_slbModuleIdMap.insert(
            std::make_pair(indexFromReadoutWithoutChannel, nullptr));
        return nullptr;
    }

    std::unique_ptr<TGCModuleId> slb{};
    for (auto& [id, module] : slbMap) {
        const auto* SLB = dynamic_cast<TGCModuleSLB*>(module.get());
        if (SLB && SLB->getSBLoc() == sbLoc) {
            slb = std::move(module);
            break;
        }
    }

    if (!slb) {
        m_slbModuleIdMap.insert(
            std::make_pair(indexFromReadoutWithoutChannel, nullptr));
        return nullptr;  // Do not delete slb here.
    }

    return m_slbModuleIdMap
        .insert(std::make_pair(indexFromReadoutWithoutChannel, std::move(slb)))
        .first->second.get();
}

// SSW ID/Rx ID -> SLB Module
std::unique_ptr<TGCModuleId> TGCCabling::getSLBFromRxId(TGCId::SideType side,
                                                        int rodId, int sswId,
                                                        int rxId) const {

    // ROD Module
    int readoutSector = rodId - 1;  // rodID = 1..12
    TGCModuleROD rod(side, readoutSector);

    // SSW Module
    TGCModuleMap sswMap = getModule(rod, TGCModuleId::SSW);
    if (sswMap.empty()) {
        return nullptr;
    }

    std::unique_ptr<TGCModuleId> ssw = sswMap.popModule(sswId);

    if (!ssw) {
        return nullptr;  // Do not need to delete ssw here.
                         // We can delete ssw but nothing will be done.
    }

    // SLB Module
    TGCModuleMap slbMap = getModule(*ssw, TGCModuleId::SLB);
    if (slbMap.empty()) {
        return nullptr;
    }

    return slbMap.popModule(rxId);
}

// SLB Module -> readout ID
bool TGCCabling::getReadoutFromSLB(const TGCModuleSLB& slb,
                                   TGCId::SideType& side, int& rodId,
                                   int& sswId, int& sbLoc) const {
    // initialize
    side = TGCId::NoSideType;
    rodId = -1;
    sswId = -1;
    sbLoc = -1;

    // Fill side
    side = slb.getSideType();

    TGCModuleMap sswMap = getModule(slb, TGCModuleId::SSW);

    if (sswMap.empty()) {
        return false;
    }

    // SSW Module
    std::unique_ptr<TGCModuleId>& ssw = sswMap.begin()->second;
    if (!ssw) {
        return false;
    }

    // Fill SSW ID
    sswId = ssw->getId();

    // Fill SBLoc
    sbLoc = slb.getSBLoc();

    if (sbLoc < 0) {
        TGCModuleMap slbMap = getModule(*ssw, TGCModuleId::SLB);
        if (slbMap.empty()) {
            return false;
        }

        TGCModuleSLB* pSlb = nullptr;

        for (auto& [id, module] : slbMap) {
            pSlb = dynamic_cast<TGCModuleSLB*>(module.get());

            if (pSlb && slb.getRegionType() == pSlb->getRegionType() &&
                slb.getSector() == pSlb->getSector() &&
                slb.getId() == pSlb->getId()) {
                if (slb.getModuleType() == pSlb->getModuleType()) {
                    sbLoc = pSlb->getSBLoc();
                    break;
                }
                // SI is connected to the SLB corrsponding WI
                if (slb.getModuleType() == TGCId::SI &&
                    pSlb->getModuleType() == TGCId::WI) {
                    sbLoc = pSlb->getSBLoc();

                    break;
                }
            }
        }
        if (sbLoc < 0) {
            return false;
        }
    }

    TGCModuleMap rodMap = getModule(*ssw, TGCModuleId::ROD);
    if (rodMap.empty()) {
        return false;
    }

    // ROD Module
    std::unique_ptr<TGCModuleId>& rod = rodMap.begin()->second;

    // Fill ROD ID
    rodId = rod->getId();
    return true;
}

// coincidence channel -> readout channel
bool TGCCabling::getReadoutFromHighPtID(
    TGCId::SideType side, int rodId, int& sswId, int& sbLoc, int& channel,
    TGCId::SignalType signal, TGCId::RegionType region, int sectorInReadout,
    int hpbId, int block, int hitId, int pos, TGCId::ModuleType moduleType,
    bool orChannel) const {
    // initialize
    sswId = -1;
    sbLoc = -1;
    channel = -1;

    // get sector number

    int readoutSector = (rodId - 1);
    int sector = sectorInReadout;
    if (rodId < 13) {
        if (region == TGCId::Forward) {
            sector +=
                readoutSector * (TGCId::NUM_FORWARD_SECTOR / TGCId::N_RODS);
        } else {
            sector +=
                readoutSector * (TGCId::NUM_ENDCAP_SECTOR / TGCId::N_RODS);
        }
    } else if (rodId < 20) {
        readoutSector -= 16;
        if (region == TGCId::Forward) {
            sector += readoutSector * (TGCId::NUM_FORWARD_SECTOR /
                                       TGCModuleId::NumberOfSReadoutSector);
        } else {
            sector += readoutSector * (TGCId::NUM_ENDCAP_SECTOR /
                                       TGCModuleId::NumberOfSReadoutSector);
        }
    }

    TGCChannelHPBIn hpbin(side, signal, region, sector, hpbId, block,
                          hitId * 2 + pos);
    if (!hpbin.isValid()) {
        return false;
    }

    std::unique_ptr<TGCChannelId> slbout =
        m_cableSLBToHPB->getChannelInforHPB(hpbin, moduleType, false);
    if (!slbout || !slbout->isValid()) {
        return 0;
    }
    std::unique_ptr<TGCChannelId> slbin =
        m_cableInSLB->getChannel(*slbout, orChannel);

    if (!slbin) {
        return false;
    }
    channel = slbin->getChannel();

    auto slb = slbin->getModule();
    if (!slb) {
        return false;
    }

    // SLB Module -> readout ID
    TGCId::SideType sideType;
    int rodid;  // dummy
    bool status = getReadoutFromSLB(*dynamic_cast<TGCModuleSLB*>(slb.get()),
                                    sideType, rodid, sswId, sbLoc);

    return status;
}

// readout channel -> coincidence channel
bool TGCCabling::getHighPtIDFromReadout(
    TGCId::SideType side, int rodId, int sswId, int sbLoc, int channel,
    TGCId::SignalType& signal, TGCId::RegionType& region, int& sectorInReadout,
    int& hpbId, int& block, int& hitId, int& pos) const {
    signal = TGCId::NoSignalType;
    region = TGCId::NoRegionType;
    sectorInReadout = -1;
    hpbId = -1;
    block = -1;
    hitId = -1;
    pos = -1;

    const TGCModuleId* slb = getSLBFromReadout(side, rodId, sswId, sbLoc);
    if (!slb) {
        return 0;
    }

    TGCChannelSLBIn slbin(slb->getSideType(), slb->getModuleType(),
                          slb->getRegionType(), slb->getSector(), slb->getId(),
                          channel);

    std::unique_ptr<TGCChannelId> hpbin =
        getChannel(slbin, TGCChannelId::ChannelIdType::HPBIn, false);
    if (!hpbin || !hpbin->isValid()) {
        return false;
    }
    signal = hpbin->getSignalType();
    region = hpbin->getRegionType();
    sectorInReadout = hpbin->getSectorInReadout();
    hpbId = hpbin->getId();
    block = hpbin->getBlock();
    pos = hpbin->getChannel() % 2;
    hitId = (hpbin->getChannel() - pos) / 2;

    return true;
}

// coincidence channel -> readout channel
bool TGCCabling::getReadoutFromLowPtCoincidence(TGCId::SideType side, int rodId,
                                                int sswId, int sbLoc,
                                                int& channel, int block,
                                                int pos, bool flag) const {
    bool orChannel = flag;

    const TGCModuleId* slb = getSLBFromReadout(side, rodId, sswId, sbLoc);
    if (!slb) {
        return 0;
    }

    TGCChannelSLBOut slbout(slb->getSideType(), slb->getModuleType(),
                            slb->getRegionType(), slb->getSector(),
                            slb->getId(), block, pos);

    std::unique_ptr<TGCChannelId> slbin =
        getChannel(slbout, TGCChannelId::ChannelIdType::SLBIn, orChannel);

    if (!slbin) {
        return false;
    }

    channel = slbin->getChannel();

    return true;
}

// readout channel -> chamber channel
std::unique_ptr<TGCChannelId> TGCCabling::getASDOutFromReadout(
    TGCId::SideType side, int rodId, int sswId, int sbLoc, int channel,
    bool orChannel) const {
    const TGCModuleId* slb = getSLBFromReadout(side, rodId, sswId, sbLoc);
    if (!slb) {
        return nullptr;
    }

    TGCChannelSLBIn slbin(slb->getSideType(), slb->getModuleType(),
                          slb->getRegionType(), slb->getSector(), slb->getId(),
                          channel);
    if (!slbin.isValid()) {
        return nullptr;
    }

    return getChannel(slbin, TGCChannelId::ChannelIdType::ASDOut, orChannel);
}

// chamber channel -> readout channel
bool TGCCabling::getReadoutFromASDOut(const TGCChannelASDOut& asdout,
                                      TGCId::SideType& side, int& rodId,
                                      int& sswId, int& sbLoc, int& channel,
                                      bool orChannel) const {
    // initialize
    side = TGCId::NoSideType;
    rodId = -1;
    sswId = -1;
    sbLoc = -1;
    channel = -1;

    // SLBIn channel
    std::unique_ptr<TGCChannelId> slbin{
        getChannel(asdout, TGCChannelId::ChannelIdType::SLBIn, orChannel)};

    if (!slbin) {
        return false;
    }
    channel = slbin->getChannel();

    auto slb = slbin->getModule();
    if (!slb) {
        return false;
    }

    // SLB Module -> readout ID
    return getReadoutFromSLB(*static_cast<TGCModuleSLB*>(slb.get()), side,
                             rodId, sswId, sbLoc);
}

std::unique_ptr<TGCChannelId> TGCCabling::getChannel(
    const TGCChannelId& channelId, TGCChannelId::ChannelIdType type,
    bool orChannel) const {
    switch (channelId.getChannelIdType()) {
        case TGCChannelId::ChannelIdType::ASDIn:
            if (type == TGCChannelId::ChannelIdType::ASDOut) {
                return m_cableInASD->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::SLBIn) {
                std::unique_ptr<TGCChannelId> asdout =
                    m_cableInASD->getChannel(channelId, false);
                if (!asdout || !asdout->isValid()) {

                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> ppin =
                    m_cableASDToPP->getChannel(*asdout, false);
                if (!ppin || !ppin->isValid()) {
                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> ppout =
                    m_cableInPP->getChannel(*ppin, orChannel);
                if (!ppout || !ppout->isValid()) {

                    return nullptr;
                }
                return m_cablePPToSLB->getChannel(*ppout, false);
            }
            break;
        case TGCChannelId::ChannelIdType::ASDOut:
            if (type == TGCChannelId::ChannelIdType::ASDIn) {
                return m_cableInASD->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::PPIn) {
                return m_cableASDToPP->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::SLBIn) {
                std::unique_ptr<TGCChannelId> ppin =
                    m_cableASDToPP->getChannel(channelId, false);
                if (!ppin || !ppin->isValid()) {

                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> ppout =
                    m_cableInPP->getChannel(*ppin, orChannel);
                if (!ppout || !ppout->isValid()) {

                    return nullptr;
                }
                return m_cablePPToSLB->getChannel(*ppout, false);
            }
            break;
        case TGCChannelId::ChannelIdType::PPIn:
            if (type == TGCChannelId::ChannelIdType::ASDOut) {
                return m_cableASDToPP->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::PPOut) {
                return m_cableInPP->getChannel(channelId, orChannel);
            }
            break;
        case TGCChannelId::ChannelIdType::PPOut:
            if (type == TGCChannelId::ChannelIdType::PPIn) {
                return m_cableInPP->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::SLBIn) {
                return m_cablePPToSLB->getChannel(channelId, orChannel);
            }
            break;
        case TGCChannelId::ChannelIdType::SLBIn:
            if (type == TGCChannelId::ChannelIdType::SLBOut) {
                return m_cableInSLB->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::HPBIn) {
                std::unique_ptr<TGCChannelId> slbout =
                    m_cableInSLB->getChannel(channelId, orChannel);
                if (!slbout || !slbout->isValid()) {

                    return nullptr;
                }
                return m_cableSLBToHPB->getChannel(*slbout, false);
            }
            if (type == TGCChannelId::ChannelIdType::PPOut) {
                return m_cablePPToSLB->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::ASDOut) {
                std::unique_ptr<TGCChannelId> ppout =
                    m_cablePPToSLB->getChannel(channelId, false);
                if (!ppout || !ppout->isValid()) {

                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> ppin =
                    m_cableInPP->getChannel(*ppout, orChannel);
                if (!ppin || !ppin->isValid()) {
                    return nullptr;
                }
                return m_cableASDToPP->getChannel(*ppin, false);
            }
            if (type == TGCChannelId::ChannelIdType::ASDIn) {
                std::unique_ptr<TGCChannelId> ppout =
                    m_cablePPToSLB->getChannel(channelId, false);
                if (!ppout || !ppout->isValid()) {
                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> ppin =
                    m_cableInPP->getChannel(*ppout, orChannel);
                if (!ppin || !ppin->isValid()) {
                    return nullptr;
                }
                std::unique_ptr<TGCChannelId> asdout =
                    m_cableASDToPP->getChannel(*ppin, false);
                if (!asdout || !asdout->isValid()) {
                    return nullptr;
                }
                return m_cableInASD->getChannel(*asdout, false);
            }
            break;
        case TGCChannelId::ChannelIdType::SLBOut:
            if (type == TGCChannelId::ChannelIdType::SLBIn) {
                return m_cableInSLB->getChannel(channelId, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::HPBIn) {
                return m_cableSLBToHPB->getChannel(channelId, orChannel);
            }
            break;
        case TGCChannelId::ChannelIdType::HPBIn:
            if (type == TGCChannelId::ChannelIdType::SLBIn) {
                std::unique_ptr<TGCChannelId> slbout =
                    m_cableSLBToHPB->getChannel(channelId, false);
                if (!slbout || !slbout->isValid()) {
                    return nullptr;
                }
                return m_cableInSLB->getChannel(*slbout, orChannel);
            }
            if (type == TGCChannelId::ChannelIdType::SLBOut) {
                return m_cableSLBToHPB->getChannel(channelId, orChannel);
            }
            break;
        default:
            break;
    }
    return nullptr;
}

TGCModuleMap TGCCabling::getModule(const TGCModuleId& moduleId,
                                   TGCModuleId::ModuleIdType type) const {
    switch (moduleId.getModuleIdType()) {
        case TGCModuleId::PP:
            if (type == TGCModuleId::SLB) {
                return m_cablePPToSLB->getModule(moduleId);
            }
            break;
        case TGCModuleId::SLB:
            if (type == TGCModuleId::PP) {
                return m_cablePPToSLB->getModule(moduleId);
            }
            if (type == TGCModuleId::HPB) {
                return m_cableSLBToHPB->getModule(moduleId);
            }
            if (type == TGCModuleId::SSW) {
                return m_cableSLBToSSW->getModule(moduleId);
            }
            break;
        case TGCModuleId::HPB:
            if (type == TGCModuleId::SLB) {
                return m_cableSLBToHPB->getModule(moduleId);
            }
            if (type == TGCModuleId::SL) {
                return m_cableHPBToSL->getModule(moduleId);
            }
            break;
        case TGCModuleId::SL:
            if (type == TGCModuleId::HPB) {
                return m_cableHPBToSL->getModule(moduleId);
            }
            break;
        case TGCModuleId::SSW:
            if (type == TGCModuleId::SLB) {
                return m_cableSLBToSSW->getModule(moduleId);
            }
            if (type == TGCModuleId::ROD) {
                return m_cableSSWToROD->getModule(moduleId);
            }
            break;
        case TGCModuleId::ROD:
            if (type == TGCModuleId::SSW) {
                return m_cableSSWToROD->getModule(moduleId);
            }
            break;
        default:
            break;
    }
    return TGCModuleMap{};
}

int TGCCabling::getIndexFromReadoutWithoutChannel(const TGCId::SideType side,
                                                  const int rodId,
                                                  const int sswId,
                                                  const int sbLoc) const {
    return ((((side - TGCId::Aside) * (MAXRODID - MINRODID + 1) + rodId -
              MINRODID) *
                 (MAXSSWID - MINSSWID + 1) +
             sswId - MINSSWID) *
                (MAXSBLOC - MINSBLOC + 1) +
            sbLoc - MINSBLOC);
}

}  // namespace MuonTGC_Cabling
