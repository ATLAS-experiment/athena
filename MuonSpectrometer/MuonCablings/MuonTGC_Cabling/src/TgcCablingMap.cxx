/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTGC_Cabling/TgcCablingMap.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TGCChannelASDIn.h"
#include "MuonTGC_Cabling/TGCChannelASDOut.h"
#include "MuonTGC_Cabling/TGCModuleMap.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"
#include "PathResolver/PathResolver.h"

using namespace MuonTGC_Cabling;
namespace Muon {

TgcCablingMap::TgcCablingMap(const Config& cfg)
    : AthMessaging{"TgcCablingMap"},
      TGCCabling{cfg},
      m_idHelperSvc{cfg.idHelperSvc},
      m_AsideId{cfg.AsideId},
      m_CsideId{cfg.CsideId} {}

///////////////////////////////////////////////////////////////
void TgcCablingMap::getReadoutIDRanges(int& maxRodId, int& maxSRodId,
                                       int& maxSswId, int& maxSbloc,
                                       int& minChannelId,
                                       int& maxChannelId) const {
    maxRodId = TGCCabling::MAXRODID;
    maxSRodId = TGCCabling::MAXSRODID;
    maxSswId = TGCCabling::MAXSSWID;
    maxSbloc = TGCCabling::MAXSBLOC;
    minChannelId = TGCCabling::MINCHANNELID;
    maxChannelId = TGCCabling::MAXCHANNELID;
}

///////////////////////////////////////////////////////////////
bool TgcCablingMap::getCoveragefromSRodID(const int srodID,
                                          int& startEndcapSector,
                                          int& coverageOfEndcapSector,
                                          int& startForwardSector,
                                          int& coverageOfForwardSector) const {
    int sectorInReadout = srodID - 17;  // srodID = 17..19
    if (sectorInReadout >= TGCModuleId::NumberOfSReadoutSector) {
        return false;
    }

    coverageOfEndcapSector =
        TGCId::NUM_ENDCAP_SECTOR / TGCModuleId::NumberOfSReadoutSector;
    startEndcapSector = sectorInReadout * coverageOfEndcapSector;
    coverageOfForwardSector =
        TGCId::NUM_FORWARD_SECTOR / TGCModuleId::NumberOfSReadoutSector;
    startForwardSector = sectorInReadout * coverageOfForwardSector;

    return true;
}

///////////////////////////////////////////////////////////////
// Readout ID is ored
bool TgcCablingMap::isOredChannel(const int subDetectorID, const int rodID,
                                  const int sswID, const int sbLoc,
                                  const int channelID) const {
    Identifier id;
    return getOfflineIDfromReadoutID(id, subDetectorID, rodID, sswID, sbLoc,
                                     channelID, true);
}

///////////////////////////////////////////////////////////////
// Offline ID has adjacent Readout ID
bool TgcCablingMap::hasAdjacentChannel(const Identifier& offlineID) const {
    int subDetectorID{0}, rodID{0}, sswID{0}, sbLoc{0}, channelID{0};
    return getReadoutIDfromOfflineID(offlineID, subDetectorID, rodID, sswID,
                                     sbLoc, channelID, true);
}

///////////////////////////////////////////////////////////////
// readout IDs -> offline IDs
bool TgcCablingMap::getOfflineIDfromReadoutID(Identifier& offlineID,
                                              const int subDetectorID,
                                              const int rodID, const int sswID,
                                              const int sbLoc,
                                              const int channelID,
                                              bool orChannel) const {
    // Readout ID -> Online ID
    int subsystemNumber{0}, octantNumber{0}, moduleNumber{0}, layerNumber{0},
        rNumber{0}, wireOrStrip{0}, channelNumber{0};

    bool status = getOnlineIDfromReadoutID(
        subDetectorID, rodID, sswID, sbLoc, channelID, subsystemNumber,
        octantNumber, moduleNumber, layerNumber, rNumber, wireOrStrip,
        channelNumber, orChannel);
    if ((!status) && (!orChannel)) {
        ATH_MSG_VERBOSE(" getOfflineIDfromReadoutID :"
                        << " Cannot get OnineID for "
                        << " subdetectorID=" << subDetectorID
                        << " rodID=" << rodID << " sswID=" << sswID
                        << " sbLoc=" << sbLoc << " channel=" << channelID
                        << " [ Or =" << orChannel << "] ");
    }
    if (!status) {
        return status;
    }

    // OnlineID -> OfflineID
    status = getOfflineIDfromOnlineID(offlineID, subsystemNumber, octantNumber,
                                      moduleNumber, layerNumber, rNumber,
                                      wireOrStrip, channelNumber);

    if (!status) {
        ATH_MSG_VERBOSE(" getOfflineIDfromReadoutID :"
                        << " Cannot get OfflineID for "
                        << " side=" << subsystemNumber << " octant="
                        << octantNumber << " module=" << moduleNumber
                        << " layer=" << layerNumber << " chamber=" << rNumber
                        << " w/s=" << wireOrStrip
                        << " channel=" << channelNumber);
    }

    return status;
}

///////////////////////////////////////////////////////////////
// offline IDs -> readout IDs
bool TgcCablingMap::getReadoutIDfromOfflineID(const Identifier& offlineID,
                                              int& subDetectorID, int& rodID,
                                              int& sswID, int& sbLoc,
                                              int& channelID,
                                              bool adChannel) const {
    int subsystemNumber;
    int octantNumber;
    int moduleNumber;
    int layerNumber;
    int rNumber;
    int wireOrStrip;
    int channelNumber;

    // OfflineID -> OnlineID
    bool status = getOnlineIDfromOfflineID(
        offlineID, subsystemNumber, octantNumber, moduleNumber, layerNumber,
        rNumber, wireOrStrip, channelNumber);

    if (!status) {
        ATH_MSG_WARNING(" Fail to getOnlineIDfromOfflineID "
                        << " for OfflineID="
                        << m_idHelperSvc->toString(offlineID));
        return status;
    }

    ATH_MSG_DEBUG("getOnlineIDfromOfflineID() "
                  << " offlineID=" << offlineID << " onlineID:  "
                  << " side=" << subsystemNumber << " octant=" << octantNumber
                  << " module=" << moduleNumber << " layer=" << layerNumber
                  << " chamber=" << rNumber << " w/s=" << wireOrStrip
                  << " channel=" << channelNumber);

    // OnlineID -> ReadoutID
    status = getReadoutIDfromOnlineID(subDetectorID, rodID, sswID, sbLoc,
                                      channelID, subsystemNumber, octantNumber,
                                      moduleNumber, layerNumber, rNumber,
                                      wireOrStrip, channelNumber, adChannel);

    if (!status) {
        if (adChannel) {
            ATH_MSG_DEBUG(" Fail to getReadoutIDfromOnlineID");
            ATH_MSG_DEBUG(" side=" << subsystemNumber << " octant="
                                   << octantNumber << " module=" << moduleNumber
                                   << " layer=" << layerNumber << " chamber="
                                   << rNumber << " w/s=" << wireOrStrip
                                   << " channel=" << channelNumber);
        } else {
            ATH_MSG_WARNING(" getReadoutIDfromOnlineID: "
                            << " Cannot get ReadoutID for "
                            << " side=" << subsystemNumber << " octant="
                            << octantNumber << " module=" << moduleNumber
                            << " layer=" << layerNumber
                            << " chamber=" << rNumber << " w/s=" << wireOrStrip
                            << " channel=" << channelNumber);
        }
    } else {
        ATH_MSG_DEBUG(" SUCCESS  getReadoutIDfromOnlineID");
        ATH_MSG_DEBUG(" side=" << subsystemNumber << " octant=" << octantNumber
                               << " module=" << moduleNumber << " layer="
                               << layerNumber << " chamber=" << rNumber
                               << " w/s=" << wireOrStrip << " channel="
                               << channelNumber << "adjacent=" << adChannel);
        ATH_MSG_DEBUG(" subDetectorID" << subDetectorID << " rodID=" << rodID
                                       << " sswID=" << sswID
                                       << " channelID=" << channelID);
    }

    return status;
}

///////////////////////////////////////////////////////////////
// offline ID -> online IDs
bool TgcCablingMap::getOnlineIDfromOfflineID(
    const Identifier& offlineId, int& subSystemNumber, int& octantNumber,
    int& moduleNumber, int& layerNumber, int& rNumber, int& wireOrStrip,
    int& channelNumber) const {
    // get station name in string format : T1F,T1E,T2F...
    const int iStation = m_idHelperSvc->tgcIdHelper().stationName(offlineId);
    const int stationType = (iStation - 39) / 2;

    if ((stationType < 1) || (stationType > 4)) {
        return false;
    }

    // eta and phi
    int iEta = m_idHelperSvc->tgcIdHelper().stationEta(offlineId);
    int iPhi = m_idHelperSvc->tgcIdHelper().stationPhi(offlineId);

    // forward/endcap
    enum { FORWARD, ENDCAP };
    const int regionType =
        m_idHelperSvc->tgcIdHelper().isForward(offlineId) ? FORWARD : ENDCAP;

    // octant index and module index
    const int sectorEI[] = {-1, 1,  2,  3,  4,  5,  6,  7,  8,  10, 11,
                            12, 13, 14, 15, 16, 18, 19, 20, 22, 23, 0};
    int sector = -1;
    if (regionType == ENDCAP) {
        if (stationType != 4) {
            // Endcap
            sector = (iPhi + 1) % TGCId::NUM_ENDCAP_SECTOR;
        } else {
            // EI
            sector = sectorEI[iPhi];
            // iPhi is redefined for internal use
            if (sector == 0) {
                iPhi = sector + TGCId::NUM_INNER_SECTOR;
            } else {
                iPhi = sector;
            }
        }
    } else {
        if (stationType != 4) {
            // Forward
            sector = iPhi % TGCId::NUM_FORWARD_SECTOR;
        } else {
            // FI
            sector = iPhi % TGCId::NUM_INNER_SECTOR;
        }
    }
    int max_module = (regionType == FORWARD || stationType == 4)
                         ? TGCId::NUM_FORWARD_SECTOR
                         : TGCId::NUM_ENDCAP_SECTOR;
    max_module /= TGCId::NUM_OCTANT;
    const int iOctant = static_cast<int>(sector / max_module);
    const int iModule = sector % max_module;

    // R index
    const int rIndex = std::abs(iEta);

    // Gas gap
    const int iGasGap = m_idHelperSvc->tgcIdHelper().gasGap(offlineId);

    // convert to ASD-Out index

    subSystemNumber = (iEta > 0) ? 1 : -1;

    octantNumber = iOctant;

    // module# convention
    //         <---- phi ----
    //
    //     7 6 4 3 1 0        11 10  9
    //      8   5   2         14 13 12
    //
    //     [M1,M2, M3]         [EI/FI]

    // module mapping        N/A  A   B   C  D  E  F
    const int modmapE[7] = {-1, 0, 1, 3, 4, 6, 7};
    const int modmapF[4] = {-1, 2, 5, 8};
    const int modmapEI[4] = {-1, 9, 10, 11};
    const int modmapFI[4] = {-1, 12, 13, 14};

    if (stationType != 4) {  // T1, T2, T3
        if (regionType == ENDCAP) {
            moduleNumber = modmapE[iModule + 1];
            rNumber = 5 - rIndex;
        } else {
            moduleNumber = modmapF[iModule + 1];
            rNumber = rIndex - 1;
        }
    } else {  // TI
        if (regionType == ENDCAP) {
            moduleNumber = modmapEI[iModule + 1];
            rNumber = rIndex - 1;  // 0-offset
        } else {
            moduleNumber = modmapFI[iModule + 1];
            rNumber = rIndex - 1;
        }
    }

    // layer#
    // 0, (1), 2, ... 5, 6 (pivot) / 7, 8, 9 (EI/FI), Layer 9 only for Run-4+
    //                        N/A  T1  T2  T3  TI
    const int lyr_offset[5] = {0, -1, 2, 4, 6};
    layerNumber = iGasGap + lyr_offset[stationType];

    // wire (0) or strip (1)
    wireOrStrip = m_idHelperSvc->tgcIdHelper().isStrip(offlineId);

    // Offline ID channel
    int channel = m_idHelperSvc->tgcIdHelper().channel(offlineId);

    // Offline ID cahnnel -> Online ID channel
    // T11S : EI @ phi=2,11,13,14,15,19,20,21
    //  total number of wire channel = 16
    //  (T11 : 24 channel)
    const int OffsetForT11S = 8;
    // T10S : FI @ phi =2,5,8,11,14,17,20,23
    //  total number of wire channel = 30
    //  (T10 32 channel)

    if ((wireOrStrip == 0) && (stationType == 4)) {  // TI wire
        if (regionType == ENDCAP) {                  // EI
            // iPhi here is onlineID from 1 to 24.
            switch (iPhi) {
                case 2:
                case 12:
                case 14:
                case 15:
                case 16:
                case 22:
                case 23:
                case 24:
                    // T11S
                    channel += OffsetForT11S;
                    break;
                default:
                    break;
            }
        }
    }
    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subSystemNumber == 1) {
        sideType = TGCId::SideType::Aside;
    }
    if (subSystemNumber == -1) {
        sideType = TGCId::SideType::Cside;
    }

    // SignalType
    TGCId::SignalType signalType = TGCId::SignalType::Undefined;
    if (wireOrStrip == 0) {
        signalType = TGCId::SignalType::Wire;
    }
    if (wireOrStrip == 1) {
        signalType = TGCId::SignalType::Strip;
    }

    // RegionType
    TGCId::RegionType region = TGCId::RegionType::Undefined;
    if (regionType == FORWARD) {
        region = TGCId::RegionType::Forward;
    }
    if (regionType == ENDCAP) {
        region = TGCId::RegionType::Endcap;
    }

    // ASDIn
    TGCChannelASDIn asdin(sideType, signalType, region, iPhi, layerNumber,
                          rIndex, channel);
    if (!asdin.isValid()) {
        return false;
    }

    // offline Id -> online Id
    std::unique_ptr<TGCChannelId> asdout =
        getChannel(asdin, TGCChannelId::ChannelIdType::ASDOut, false);
    if (asdout == nullptr || !asdout->isValid()) {
        return false;
    }
    channelNumber = asdout->getChannel();

    return true;
}

///////////////////////////////////////////////////////////////
// online IDs -> offline ID
bool TgcCablingMap::getOfflineIDfromOnlineID(
    Identifier& offlineId, const int subSystemNumber, const int octantNumber,
    const int moduleNumber, const int layerNumber, const int rNumber,
    const int wireOrStrip, const int channelNumber) const {
    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subSystemNumber == 1) {
        sideType = TGCId::SideType::Aside;
    } else if (subSystemNumber == -1) {
        sideType = TGCId::SideType::Cside;
    }

    // SignalType
    TGCId::SignalType signalType = TGCId::SignalType::Undefined;
    if (wireOrStrip == 0) {
        signalType = TGCId::SignalType::Wire;
    }
    if (wireOrStrip == 1) {
        signalType = TGCId::SignalType::Strip;
    }

    ATH_MSG_VERBOSE("getOfflineIDfromOnlineID for "
                    << " side=" << sideType << " octant=" << octantNumber
                    << " module=" << moduleNumber << " layer=" << layerNumber
                    << " chamber=" << rNumber << " w/s=" << signalType
                    << " channel=" << channelNumber);

    // ASDOut
    TGCChannelASDOut asdout(sideType, signalType, octantNumber, moduleNumber,
                            layerNumber, rNumber, channelNumber);
    if (!asdout.isValid()) {
        ATH_MSG_WARNING(" Illegal AsdOut for "
                        << " side=" << sideType << " octant=" << octantNumber
                        << " module=" << moduleNumber
                        << " layer=" << layerNumber << " chamber=" << rNumber
                        << " w/s=" << signalType
                        << " channel=" << channelNumber);
        return false;
    }

    // online Id -> offline Id
    std::unique_ptr<TGCChannelId> asdin =
        getChannel(asdout, TGCChannelId::ChannelIdType::ASDIn, false);
    if (!asdin || !asdin->isValid()) {
        ATH_MSG_WARNING(" getOfflineIDfromOnlineID : Illegal AsdIn for "
                        << " side=" << sideType << " octant=" << octantNumber
                        << " module=" << moduleNumber
                        << " layer=" << layerNumber << " chamber=" << rNumber
                        << " w/s=" << signalType
                        << " channel=" << channelNumber);
        return false;
    }

    // build identifier
    std::string stationNameStr;
    switch (asdin->getStation()) {
        case TGCId::StationType::M1:  // Triplet
            stationNameStr = (asdin->isForward()) ? "T1F" : "T1E";
            break;
        case TGCId::StationType::M2:  // Middle Doublet
            stationNameStr = (asdin->isForward()) ? "T2F" : "T2E";
            break;
        case TGCId::StationType::M3:  // Pivot Doublet
            stationNameStr = (asdin->isForward()) ? "T3F" : "T3E";
            break;
        case TGCId::StationType::M4:  // Inner
            stationNameStr = (asdin->isForward()) ? "T4F" : "T4E";
            break;
        default:
            return false;
    }
    int stationEta = asdin->getChamber();
    if (asdin->getSideType() == TGCId::SideType::Cside) {
        stationEta *= -1;
    }
    int stationPhi = asdin->getSector();
    int gasGap = asdin->getGasGap();
    int isStrip = (asdin->isStrip()) ? 1 : 0;
    int channel = asdin->getChannel();

    if ((asdin->getStation() == TGCId::StationType::M4) && (asdin->isEndcap())) {
        // special treatment for EI
        constexpr std::array<int, 25> phiIE{-1, 1,  2,  3,  4,  5,  6,  7,  8,
                                            -1, 9,  10, 11, 12, 13, 14, 15, -1,
                                            16, 17, 18, -1, 19, 20, 21};
        stationPhi = phiIE[asdin->getSector()];
        if (stationPhi < 0) {
            return false;
        }
    }

    // OnlineID --> OfflineID
    // T11S : EI @ stationPhi=2,11,13,14,15,19,20,21
    //  total number of wire channel = 16
    //  (T11  24 channel)
    const int OffsetForT11S = 8;
    // T10S : FI @ statioPhi =2,5,8,11,14,17,20,23
    //  total number of wire channel = 30
    //  (T10  32 channel)
    if ((asdin->getStation() == TGCId::StationType::M4) && (!isStrip)) {  //  Inner Wire
        if (asdin->isEndcap()) {
            switch (stationPhi) {
                case 2:
                case 11:
                case 13:
                case 14:
                case 15:
                case 19:
                case 20:
                case 21:
                    // T11S
                    channel -= OffsetForT11S;
                    if (channel <= 0) {
                        return false;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    offlineId = m_idHelperSvc->tgcIdHelper().channelID(
        stationNameStr, stationEta, stationPhi, gasGap, isStrip, channel);

    return true;
}
///////////////////////////////////////////////////////////////
// readout IDs -> online IDs
bool TgcCablingMap::getOnlineIDfromReadoutID(
    const int subDetectorID, const int rodID, const int sswID, const int sbLoc,
    const int channelID, int& subsystemNumber, int& octantNumber,
    int& moduleNumber, int& layerNumber, int& rNumber, int& wireOrStrip,
    int& channelNumber, bool orChannel) const {
    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subDetectorID == m_AsideId) {
        sideType = TGCId::SideType::Aside;
    } else if (subDetectorID == m_CsideId) {
        sideType = TGCId::SideType::Cside;
    }

    // readout channel -> chamber channel
    std::unique_ptr<TGCChannelId> asdout = getASDOutFromReadout(
        sideType, rodID, sswID, sbLoc, channelID, orChannel);
    if (asdout == nullptr) {
        if (!orChannel) {
            ATH_MSG_VERBOSE(" getOnlineIDfromReadoutID :"
                            << " Cannot get OnlineID of "
                            << " side=" << sideType << " rodID=" << rodID
                            << " sswID=" << sswID << " sbLoc=" << sbLoc
                            << " channel=" << channelID
                            << " [ Or =" << orChannel << "] ");
        }
        return false;
    }
    if (!asdout->isValid()) {
        ATH_MSG_WARNING(" getOnlineIDfromReadoutID :"
                        << " Illegal ASDout for "
                        << " side=" << sideType << " rodID=" << rodID
                        << " sswID=" << sswID << " sbLoc=" << sbLoc
                        << " channel=" << channelID << " [ Or =" << orChannel
                        << "] ");

        return false;
    }
    // SubsystemNumber
    subsystemNumber = (asdout->isAside()) ? 1 : -1;

    // OctantNumber
    octantNumber = asdout->getOctant();

    // RNumber
    rNumber = asdout->getChamber();

    // ModuleNumber
    moduleNumber = asdout->getSectorModule();

    layerNumber = asdout->getLayer();

    // WireOrStrip
    wireOrStrip = (asdout->isStrip()) ? 1 : 0;

    // ChannelNumber
    channelNumber = asdout->getChannel();

    return true;
}

//////////////////////////////////////////////////////////
// online IDs -> readout IDs
bool TgcCablingMap::getReadoutIDfromOnlineID(
    int& subDetectorID, int& rodID, int& sswID, int& sbLoc, int& channelID,
    const int subsystemNumber, const int octantNumber, const int moduleNumber,
    const int layerNumber, const int rNumber, const int wireOrStrip,
    const int channelNumber, bool adChannel) const {
    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subsystemNumber == 1) {
        sideType = TGCId::SideType::Aside;
    }
    if (subsystemNumber == -1) {
        sideType = TGCId::SideType::Cside;
    }

    // SignalType
    TGCId::SignalType signalType = TGCId::SignalType::Undefined;
    if (wireOrStrip == 0) {
        signalType = TGCId::SignalType::Wire;
    }
    if (wireOrStrip == 1) {
        signalType = TGCId::SignalType::Strip;
    }

    // ASDOut
    TGCChannelASDOut asdout(sideType, signalType, octantNumber, moduleNumber,
                            layerNumber, rNumber, channelNumber);
    if (!asdout.isValid()) {
        ATH_MSG_WARNING(" getReadoutIDfromOnlineID() :"
                        << " Cannot get ASDout for  "
                        << " side=" << sideType << " signal=" << signalType
                        << " octant=" << octantNumber
                        << " module=" << moduleNumber
                        << " layer=" << layerNumber << " chamber=" << rNumber
                        << " channel=" << channelNumber);
    }

    if (!asdout.isValid()) {
        return false;
    }

    // chamber channel -> readout channel
    bool status = getReadoutFromASDOut(asdout, sideType, rodID, sswID, sbLoc,
                                       channelID, adChannel);
    if (!status) {
        if (adChannel) {
            ATH_MSG_DEBUG("getReadoutIDfromASDOut fails for adjacent");
        } else {
            ATH_MSG_WARNING(" getReadoutIDfromASDOut :"
                            << " Cannot get ReadoutID for "
                            << " side=" << sideType << " signal=" << signalType
                            << " octant=" << octantNumber << " module="
                            << moduleNumber << " layer=" << layerNumber
                            << " chamber=" << rNumber
                            << " channel=" << channelNumber);
        }
        return false;
    }

    // SubDetectorID
    if (sideType == TGCId::SideType::Aside) {
        subDetectorID = m_AsideId;
    }
    if (sideType == TGCId::SideType::Cside) {
        subDetectorID = m_CsideId;
    }

    return status;
}

// element ID -> readout IDs
bool TgcCablingMap::getReadoutIDfromElementID(const Identifier& elementID,
                                              int& subdetectorID,
                                              int& rodID) const {
    // get station name in string format : T1F,T1E,T2F...
    const int iStation = m_idHelperSvc->tgcIdHelper().stationName(elementID);
    const int stationType = (iStation - 39) / 2;

    int iEta = m_idHelperSvc->tgcIdHelper().stationEta(elementID);
    int iPhi = m_idHelperSvc->tgcIdHelper().stationPhi(elementID);

    // forward/endcap
    enum { FORWARD, ENDCAP };
    const int regionType =
        m_idHelperSvc->tgcIdHelper().isForward(elementID) ? FORWARD : ENDCAP;

    // SideType
    subdetectorID = (iEta > 0) ? m_AsideId : m_CsideId;

    // sector index and module index
    const int max_phi = (regionType == FORWARD || stationType == 4)
                            ? TGCId::NUM_FORWARD_SECTOR
                            : TGCId::NUM_ENDCAP_SECTOR;
    const int max_module = max_phi / TGCId::N_RODS;
    const int sector = ((iPhi - 1) + max_phi / 24 + max_phi) % max_phi;
    int readoutSector = static_cast<int>(sector / max_module);
    // Inner case
    if (stationType == 4) {
        readoutSector = (readoutSector / 3);
        readoutSector = 3 * readoutSector + 1;
    }

    rodID = readoutSector + 1;

    return true;
}

///////////////////////////////////////////////////////////////
// readout IDs -> element ID
bool TgcCablingMap::getElementIDfromReadoutID(Identifier& elementID,
                                              const int subDetectorID,
                                              const int rodID, const int sswID,
                                              const int sbLoc,
                                              const int channelID,
                                              bool orChannel) const {
    Identifier offlineID;

    // get min/max values for ReadoutID parameters
    int maxRodId, maxSRodId, maxSswId, maxSbloc, minChannelId, maxChannelId;
    getReadoutIDRanges(maxRodId, maxSRodId, maxSswId, maxSbloc, minChannelId,
                       maxChannelId);

    // check sswID and channelID in allowed range
    if ((sswID > maxSswId) || (channelID < minChannelId) ||
        (channelID > maxChannelId)) {
        ATH_MSG_DEBUG(" getElementIDfromReadoutID() :"
                      << " Illeagal channel ID"
                      << " subdetectorID=" << subDetectorID
                      << " rodID=" << rodID << " sswID=" << sswID
                      << " sbLoc=" << sbLoc << " channel=" << channelID
                      << " [ Or =" << orChannel << "] ");
        return false;
    }

    bool status = getOfflineIDfromReadoutID(offlineID, subDetectorID, rodID,
                                            sswID, sbLoc, channelID, orChannel);
    if (!status) {
        ATH_MSG_DEBUG(
            " getElementIDfromReadoutID :" << " Cannot get OfflineID ");
        return false;
    }

    elementID = m_idHelperSvc->tgcIdHelper().elementID(offlineID);
    return true;
}

///////////////////////////////////////////////////////////////
// readout ID -> SLB ID
bool TgcCablingMap::getSLBIDfromReadoutID(int& phi, bool& isAside,
                                          bool& isEndcap, int& moduleType,
                                          int& id, const int subsectorID,
                                          const int rodID, const int sswID,
                                          const int sbLoc) const {
    isAside = (subsectorID == m_AsideId);

    TGCId::SideType side = isAside ? TGCId::SideType::Aside : TGCId::SideType::Cside;

    const TGCModuleId* slb = getSLBFromReadout(side, rodID, sswID, sbLoc);
    if (!slb) {
        ATH_MSG_VERBOSE(" getSLBIDfromReadoutID :"
                        << " Cannot get SLB of "
                        << " side" << side << " rodID=" << rodID
                        << " sswID=" << sswID << " sbLoc=" << sbLoc);
        return false;
    }

    isEndcap = (slb->getRegionType() == TGCId::RegionType::Endcap);
    moduleType = (int)slb->getModuleType();
    bool isInner = (moduleType == +TGCId::ModuleType::WI || moduleType == +TGCId::ModuleType::SI);
    int offset, numOfSector;
    if (isInner) {
        numOfSector = TGCId::NUM_INNER_SECTOR;
    } else {
        if (isEndcap) {
            numOfSector = TGCId::NUM_ENDCAP_SECTOR;
        } else {
            numOfSector = TGCId::NUM_FORWARD_SECTOR;
        }
    }
    offset = numOfSector - numOfSector / 24;
    phi = (slb->getSector() + offset) % numOfSector + 1;
    id = slb->getId();

    return true;
}

///////////////////////////////////////////////////////////////
// readout ID -> rxID
bool TgcCablingMap::getSLBAddressfromReadoutID(int& slbAddr,
                                               const int subsectorID,
                                               const int rodID, const int sswID,
                                               const int sbLoc) const {
    slbAddr = -1;

    bool isAside = (subsectorID == m_AsideId);

    TGCId::SideType side = isAside ? TGCId::SideType::Aside : TGCId::SideType::Cside;

    const TGCModuleId* slb = getSLBFromReadout(side, rodID, sswID, sbLoc);
    if (!slb) {
        ATH_MSG_WARNING(" getSLBAddressfromReadoutID :"
                        << " Cannot get SLB of "
                        << " side" << side << " rodID=" << rodID
                        << " sswID=" << sswID << " sbLoc=" << sbLoc);
        return false;
    }

    const TGCModuleSLB* modSlb = dynamic_cast<const TGCModuleSLB*>(slb);
    if (!modSlb) {
        ATH_MSG_WARNING("TgcCablingMap::getSLBAddressfromReadoutID "
                        << "dynamic_cast<const "
                           "TGCModuleSLB*>(slb) failed.");
        return false;
    }

    slbAddr = modSlb->getSlbAddr();
    return true;
}

///////////////////////////////////////////////////////////////
// ROD_ID / SSW_ID / RX_ID -> SLB ID
bool TgcCablingMap::getSLBIDfromRxID(int& phi, bool& isAside, bool& isEndcap,
                                     int& moduleType, int& id,
                                     const int subsectorID, const int rodID,
                                     const int sswID, const int rxId) const {
    isAside = (subsectorID == m_AsideId);
    TGCId::SideType side = isAside ? TGCId::SideType::Aside : TGCId::SideType::Cside;

    std::unique_ptr<TGCModuleId> slb = getSLBFromRxId(side, rodID, sswID, rxId);
    if (!slb) {
        ATH_MSG_WARNING("geSLBIDfromRxID :" << " Cannot get SLB of "
                                            << " side" << side << " rodID="
                                            << rodID << " sswID=" << sswID
                                            << " rxId=" << rxId);
        return false;
    }

    isEndcap = (slb->getRegionType() == TGCId::RegionType::Endcap);
    moduleType = (int)slb->getModuleType();
    bool isInner = (moduleType == +TGCId::ModuleType::WI || moduleType == +TGCId::ModuleType::SI);
    int offset, numOfSector;
    if (isInner) {
        numOfSector = TGCId::NUM_INNER_SECTOR;
    } else {
        if (isEndcap) {
            numOfSector = TGCId::NUM_ENDCAP_SECTOR;
        } else {
            numOfSector = TGCId::NUM_FORWARD_SECTOR;
        }
    }
    offset = numOfSector - numOfSector / 24;
    phi = (slb->getSector() + offset) % numOfSector + 1;
    id = slb->getId();
    return true;
}

///////////////////////////////////////////////////////////////
// SLB ID -> readout ID
bool TgcCablingMap::getReadoutIDfromSLBID(const int phi, const bool isAside,
                                          const bool isEndcap,
                                          const int moduleType, const int id,
                                          int& subsectorID, int& rodID,
                                          int& sswID, int& sbLoc) const {
    TGCId::ModuleType module = static_cast<TGCId::ModuleType>(moduleType);
    TGCId::RegionType region = isEndcap ? TGCId::RegionType::Endcap : TGCId::RegionType::Forward;
    bool isInner = (module == TGCId::ModuleType::WI || module == TGCId::ModuleType::SI);
    int sector = -1;  // sector=0-47(EC), 0-23(FWD), 0-23(INNER)
    if (isInner) {
        sector = phi % TGCId::NUM_INNER_SECTOR;
    } else if (isEndcap) {
        sector = (phi + 1) % TGCId::NUM_ENDCAP_SECTOR;
    } else {
        sector = phi % TGCId::NUM_FORWARD_SECTOR;
    }
    TGCId::SideType side = isAside ? TGCId::SideType::Aside : TGCId::SideType::Cside;

    TGCModuleSLB slb(side, module, region, sector, id);

    if (!slb.isValid()) {
        ATH_MSG_WARNING("getReadoutIDfromSLBID "
                        << " phi=" << phi << " side=" << ((isAside) ? "A" : "C")
                        << " region=" << ((isEndcap) ? "Endcap" : "Forward")
                        << " type=" << moduleType << " id=" << id
                        << " Invalid SLB");
    }
    if (!slb.isValid()) {
        return false;
    }

    subsectorID = (isAside ? m_AsideId : m_CsideId);

    bool status = getReadoutFromSLB(slb, side, rodID, sswID, sbLoc);

    if (!status) {
        ATH_MSG_DEBUG(" FAIL  getReadoutIDfromSLBID");
        return false;
    }
    ATH_MSG_DEBUG(" SUCCESS  getReadoutIDfromSLBID");
    ATH_MSG_DEBUG(" phi=" << phi 
                  << " side=" << ((isAside) ? "A" : "C")
                  << " region=" << ((isEndcap) ? "Endcap" : "Forward")
                  << " type=" << moduleType << " id=" << id << " side"
                  << side << " rodID=" << rodID << " sswID=" << sswID
                  << " sbLoc=" << sbLoc);
    return true;
}

///////////////////////////////////////////////////////////////
// readout ID -> SL ID
bool TgcCablingMap::getSLIDfromReadoutID(int& phi, bool& isAside,
                                         bool& isEndcap, const int subsectorID,
                                         const int rodID, const int sswID,
                                         const int sbLoc) const {
    isAside = (subsectorID == m_AsideId);
    if (!isAside && (subsectorID != m_CsideId)) {
        ATH_MSG_WARNING(" getSLIDfromReadoutID : "
                        << " ERROR  illegal subsectorID [=" << subsectorID
                        << "] ");
        return false;
    }
    int sectorInReadout = (rodID - 1);  // rodID = 1..12 for both sides
    if (sectorInReadout >= TGCId::N_RODS) {
        return false;
    }

    // sswID check removed
    if (sswID != 9) {
        ATH_MSG_WARNING(" getSLIDfromReadoutID : "
                        << " ERROR  sswID for SL should be 9 [now =" << sswID
                        << "] ");
        return false;
    }

    int offset, numOfSector, sector;
    if (0 <= sbLoc && sbLoc <= 3) {
        isEndcap = true;
        numOfSector = TGCId::NUM_ENDCAP_SECTOR;   // 48
        offset = numOfSector - numOfSector / 24;  // 48 - 2 = 46
        sector = numOfSector * sectorInReadout / TGCId::N_RODS;
        phi = (sector + sbLoc + offset) % numOfSector + 1;
    } else if (sbLoc == 4 || sbLoc == 5) {
        isEndcap = false;
        numOfSector = TGCId::NUM_FORWARD_SECTOR;
        offset = numOfSector - numOfSector / 24;
        sector = numOfSector * sectorInReadout / TGCId::N_RODS;
        phi = (sector + (sbLoc - 4) + offset) % numOfSector + 1;
    } else {
        ATH_MSG_WARNING(" getSLIDfromReadoutID : "
                        << " ERROR  illegal sbLoc for SL [=" << sbLoc << "] ");
        return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////
// readout ID (only SROD) -> SL ID
bool TgcCablingMap::getSLIDfromSReadoutID(int& phi, bool& isAside,
                                          const int subsectorID,
                                          const int srodID, const int sector,
                                          const bool forward) const {
    isAside = (subsectorID == m_AsideId);
    if (!isAside && (subsectorID != m_CsideId)) {
        ATH_MSG_WARNING(" getSLIDfromReadoutID : "
                        << " ERROR  illegal subsectorID [=" << subsectorID
                        << "] ");
        return false;
    }

    int sectorInReadout = (srodID - 17);  // 0-2, srodID : 0x11-0x13 (17-19)
    if ((sectorInReadout >= TGCModuleId::NumberOfSReadoutSector) ||
        (sectorInReadout < 0)) {
        ATH_MSG_WARNING(" Invalid SROD ID : " << srodID);
        return false;
    }

    int offset, tmpsector, numOfSector;
    // sswID check removed
    if (forward) {
        numOfSector = TGCId::NUM_FORWARD_SECTOR;
        offset = numOfSector - numOfSector / 24;  // 24 - 1
        tmpsector = numOfSector * sectorInReadout /
                    TGCModuleId::NumberOfSReadoutSector;  // 8*[0-2]
        phi = (sector + tmpsector + offset) % numOfSector + 1;
    } else {
        numOfSector = TGCId::NUM_ENDCAP_SECTOR;
        offset = numOfSector - numOfSector / 24;  // 48 - 2
        tmpsector = numOfSector * sectorInReadout /
                    TGCModuleId::NumberOfSReadoutSector;  // 16*[0-2]
        phi = (sector + tmpsector + offset) % numOfSector + 1;
    }
    return true;
}

///////////////////////////////////////////////////////////////
// SL ID -> readout ID
bool TgcCablingMap::getReadoutIDfromSLID(const int phi, const bool isAside,
                                         const bool isEndcap, int& subsectorID,
                                         int& rodID, int& sswID,
                                         int& sbLoc) const {
    if (isAside) {
        subsectorID = m_AsideId;
    } else {
        subsectorID = m_CsideId;
    }

    if (isEndcap) {
        if (phi < 1 || phi > TGCId::NUM_ENDCAP_SECTOR) {
            return false;
        }
    } else {
        if (phi < 1 || phi > TGCId::NUM_FORWARD_SECTOR) {
            return false;
        }
    }

    int sector;
    int sectorInReadout;
    if (isEndcap) {
        sector = (phi + 1) % TGCId::NUM_ENDCAP_SECTOR;
        sectorInReadout = sector % (TGCId::NUM_ENDCAP_SECTOR / TGCId::N_RODS);
        sbLoc = sectorInReadout;
        rodID = (sector - sectorInReadout) /
                    (TGCId::NUM_ENDCAP_SECTOR / TGCId::N_RODS) +
                1;
    } else {
        sector = phi % TGCId::NUM_FORWARD_SECTOR;
        sectorInReadout = sector % (TGCId::NUM_FORWARD_SECTOR / TGCId::N_RODS);
        sbLoc = sectorInReadout + 4;
        rodID = (sector - sectorInReadout) /
                    (TGCId::NUM_FORWARD_SECTOR / TGCId::N_RODS) +
                1;
    }
    // Fixed SSWID for SL
    sswID = 9;

    return true;
}

///////////////////////////////////////////////////////////////
// SL ID -> readout ID
bool TgcCablingMap::getSReadoutIDfromSLID(const int phi, const bool isAside,
                                          const bool isEndcap, int& subsectorID,
                                          int& srodID, int& sswID,
                                          int& sbLoc) const {
    if (isAside) {
        subsectorID = m_AsideId;
    } else {
        subsectorID = m_CsideId;
    }

    if (isEndcap) {
        if (phi < 1 || phi > TGCId::NUM_ENDCAP_SECTOR) {
            return false;
        }
    } else {
        if (phi < 1 || phi > TGCId::NUM_FORWARD_SECTOR) {
            return false;
        }
    }

    int sector;
    int sectorInReadout;
    if (isEndcap) {
        sector = (phi + 1) % TGCId::NUM_ENDCAP_SECTOR;
        sectorInReadout =
            sector %
            (TGCId::NUM_ENDCAP_SECTOR /
             TGCModuleId::NumberOfSReadoutSector);  // [2-48, 1] % (48/3)
        sbLoc = sectorInReadout;
        srodID =
            (sector - sectorInReadout) / (TGCId::NUM_ENDCAP_SECTOR /
                                          TGCModuleId::NumberOfSReadoutSector) +
            0x11;
    } else {
        sector = phi % TGCId::NUM_FORWARD_SECTOR;
        sectorInReadout =
            sector %
            (TGCId::NUM_FORWARD_SECTOR /
             TGCModuleId::NumberOfSReadoutSector);  // [2-23, 1] % (24/3)
        sbLoc = sectorInReadout;
        srodID =
            (sector - sectorInReadout) / (TGCId::NUM_FORWARD_SECTOR /
                                          TGCModuleId::NumberOfSReadoutSector) +
            0x11;
    }
    // Fixed SSWID for SL
    sswID = 9;

    return true;
}

///////////////////////////////////////////////////////////////
// HPT ID -> readout ID
bool TgcCablingMap::getReadoutIDfromHPTID(const int phi, const bool isAside,
                                          const bool isEndcap, const bool,
                                          const int, int& subsectorID,
                                          int& rodID, int& sswID,
                                          int& sbLoc) const {
    return getReadoutIDfromSLID(phi, isAside, isEndcap, subsectorID, rodID,
                                sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// CAUTION!!: return RDO value (not the value for simulation)
bool TgcCablingMap::getHighPtIDfromROINumber(int roi, bool isForward,
                                             bool isStrip, int& hpb, int& chip,
                                             int& hitId, int& sub) const {
    // for Strip, there is some ambiguity in the relation between hitID and ROI
    bool status = true;
    int RoiRow = static_cast<int>(roi / 4);
    int RoiColumn = static_cast<int>(roi % 4);

    if (!isStrip) {
        if (isForward) {
            switch (RoiRow) {
                case 0:
                    chip = 0;
                    hitId = 1;
                    sub = 0;
                    break;
                case 1:
                    chip = 0;
                    hitId = 1;
                    sub = 1;
                    break;
                case 2:
                    chip = 0;
                    hitId = 2;
                    sub = 0;
                    break;
                case 3:
                    chip = 0;
                    hitId = 2;
                    sub = 1;
                    break;
                case 4:
                    chip = 0;
                    hitId = 3;
                    sub = 0;
                    break;
                case 5:
                    chip = 0;
                    hitId = 3;
                    sub = 1;
                    break;
                case 6:
                    chip = 0;
                    hitId = 4;
                    sub = 0;
                    break;
                case 7:
                    chip = 0;
                    hitId = 4;
                    sub = 1;
                    break;
                case 8:
                    chip = 0;
                    hitId = 5;
                    sub = 0;
                    break;
                case 9:
                    chip = 0;
                    hitId = 5;
                    sub = 1;
                    break;
                case 10:
                    chip = 0;
                    hitId = 6;
                    sub = 0;
                    break;
                case 11:
                    chip = 0;
                    hitId = 6;
                    sub = 1;
                    break;
                case 12:
                    chip = 1;
                    hitId = 1;
                    sub = 0;
                    break;
                case 13:
                    chip = 1;
                    hitId = 1;
                    sub = 1;
                    break;
                case 14:
                    chip = 1;
                    hitId = 2;
                    sub = 0;
                    break;
                case 15:
                    chip = 1;
                    hitId = 2;
                    sub = 1;
                    break;
                default:
                    status = false;
                    break;
            }
        } else {
            switch (RoiRow) {
                case 0:
                    chip = 0;
                    hitId = 1;
                    sub = 1;
                    break;
                case 1:
                    chip = 1;
                    hitId = 1;
                    sub = 0;
                    break;
                case 2:
                    chip = 1;
                    hitId = 1;
                    sub = 1;
                    break;
                case 3:
                    chip = 1;
                    hitId = 2;
                    sub = 0;
                    break;
                case 4:
                    chip = 1;
                    hitId = 2;
                    sub = 1;
                    break;
                case 5:
                    chip = 1;
                    hitId = 3;
                    sub = 0;
                    break;
                case 6:
                    chip = 1;
                    hitId = 3;
                    sub = 1;
                    break;
                case 7:
                    chip = 1;
                    hitId = 4;
                    sub = 0;
                    break;
                case 8:
                    chip = 1;
                    hitId = 4;
                    sub = 1;
                    break;
                case 9:
                    chip = 1;
                    hitId = 5;
                    sub = 0;
                    break;
                case 10:
                    chip = 1;
                    hitId = 5;
                    sub = 1;
                    break;
                case 11:
                    chip = 1;
                    hitId = 6;
                    sub = 0;
                    break;
                case 12:
                    chip = 1;
                    hitId = 6;
                    sub = 1;
                    break;
                case 13:
                    chip = 2;
                    hitId = 1;
                    sub = 0;
                    break;
                case 14:
                    chip = 2;
                    hitId = 1;
                    sub = 1;
                    break;
                case 15:
                    chip = 2;
                    hitId = 2;
                    sub = 0;
                    break;
                case 16:
                    chip = 2;
                    hitId = 2;
                    sub = 1;
                    break;
                case 17:
                    chip = 2;
                    hitId = 3;
                    sub = 0;
                    break;
                case 18:
                    chip = 2;
                    hitId = 3;
                    sub = 1;
                    break;
                case 19:
                    chip = 2;
                    hitId = 4;
                    sub = 0;
                    break;
                case 20:
                    chip = 2;
                    hitId = 4;
                    sub = 1;
                    break;
                case 21:
                    chip = 2;
                    hitId = 5;
                    sub = 0;
                    break;
                case 22:
                    chip = 2;
                    hitId = 5;
                    sub = 1;
                    break;
                case 23:
                    chip = 2;
                    hitId = 6;
                    sub = 0;
                    break;
                case 24:
                    chip = 2;
                    hitId = 6;
                    sub = 1;
                    break;
                case 25:
                    chip = 3;
                    hitId = 1;
                    sub = 0;
                    break;
                case 26:
                    chip = 3;
                    hitId = 1;
                    sub = 1;
                    break;
                case 27:
                    chip = 3;
                    hitId = 2;
                    sub = 0;
                    break;
                case 28:
                    chip = 3;
                    hitId = 2;
                    sub = 1;
                    break;
                case 29:
                    chip = 3;
                    hitId = 3;
                    sub = 0;
                    break;
                case 30:
                    chip = 3;
                    hitId = 3;
                    sub = 1;
                    break;
                case 31:
                    chip = 3;
                    hitId = 4;
                    sub = 0;
                    break;
                case 32:
                    chip = 3;
                    hitId = 4;
                    sub = 1;
                    break;
                case 33:
                    chip = 3;
                    hitId = 5;
                    sub = 0;
                    break;
                case 34:
                    chip = 3;
                    hitId = 5;
                    sub = 1;
                    break;
                case 35:
                    chip = 3;
                    hitId = 6;
                    sub = 0;
                    break;
                case 36:
                    chip = 3;
                    hitId = 6;
                    sub = 1;
                    break;
                default:
                    status = false;
                    break;
            }
        }
    } else if (isStrip) {
        if (isForward) {
            switch (RoiColumn) {
                case 0:
                    chip = 0;
                    hitId = 1;
                    sub = 0;
                    break;
                case 1:
                    chip = 0;
                    hitId = 1;
                    sub = 1;
                    break;
                case 2:
                    chip = 0;
                    hitId = 2;
                    sub = 0;
                    break;
                case 3:
                    chip = 0;
                    hitId = 2;
                    sub = 1;
                    break;
                default:
                    status = false;
                    break;
            }
        } else {
            switch (RoiColumn) {
                case 0:
                    if (RoiRow < 5) {
                        chip = 0;
                        hitId = 1;
                        sub = 0;
                    } else if (RoiRow < 8) {
                        chip = 0;
                        hitId = 3;
                        sub = 0;
                    } else if (RoiRow < 12) {
                        chip = 0;
                        hitId = 5;
                        sub = 0;
                    } else if (RoiRow < 25) {
                        chip = 1;
                        hitId = 1;
                        sub = 0;
                    } else if (RoiRow < 37) {
                        chip = 1;
                        hitId = 5;
                        sub = 0;
                    } else {
                        status = false;
                    }
                    break;
                case 1:
                    if (RoiRow < 5) {
                        chip = 0;
                        hitId = 1;
                        sub = 1;
                    } else if (RoiRow < 8) {
                        chip = 0;
                        hitId = 3;
                        sub = 1;
                    } else if (RoiRow < 12) {
                        chip = 0;
                        hitId = 5;
                        sub = 1;
                    } else if (RoiRow < 25) {
                        chip = 1;
                        hitId = 1;
                        sub = 1;
                    } else if (RoiRow < 37) {
                        chip = 1;
                        hitId = 5;
                        sub = 1;
                    } else {
                        status = false;
                    }
                    break;
                case 2:
                    if (RoiRow < 5) {
                        chip = 0;
                        hitId = 2;
                        sub = 0;
                    } else if (RoiRow < 8) {
                        chip = 0;
                        hitId = 4;
                        sub = 0;
                    } else if (RoiRow < 12) {
                        chip = 0;
                        hitId = 6;
                        sub = 0;
                    } else if (RoiRow < 25) {
                        chip = 1;
                        hitId = 2;
                        sub = 0;
                    } else if (RoiRow < 37) {
                        chip = 1;
                        hitId = 6;
                        sub = 0;
                    } else {
                        status = false;
                    }
                    break;
                case 3:
                    if (RoiRow < 5) {
                        chip = 0;
                        hitId = 2;
                        sub = 1;
                    } else if (RoiRow < 8) {
                        chip = 0;
                        hitId = 4;
                        sub = 1;
                    } else if (RoiRow < 12) {
                        chip = 0;
                        hitId = 6;
                        sub = 1;
                    } else if (RoiRow < 25) {
                        chip = 1;
                        hitId = 2;
                        sub = 1;
                    } else if (RoiRow < 37) {
                        chip = 1;
                        hitId = 6;
                        sub = 1;
                    } else {
                        status = false;
                    }
                    break;
                default:
                    status = false;
                    break;
            }
        }
    }

    hpb = 0;

    return status;
}

///////////////////////////////////////////////////////////////
bool TgcCablingMap::getROINumberfromHighPtID(int& roi, bool isForward,
                                             int,  // hpb_wire
                                             int chip_wire, int hitId_wire,
                                             int sub_wire,
                                             int,  // chip_strip
                                             int hitId_strip,
                                             int sub_strip) const {
    // for Strip, there is some ambiguity in the relation between hitID and ROI
    // CAUTION!!: return RDO value (not the value for simulation)
    bool status = true;
    int RoiRow = 0;
    int RoiColumn = 0;

    if (isForward) {
        RoiRow = 12 * chip_wire + 2 * (hitId_wire - 1) + sub_wire;
        RoiColumn = 2 * ((hitId_strip - 1) % 2) + sub_strip;
    } else if (!isForward) {
        RoiColumn = 2 * ((hitId_strip - 1) % 2) + sub_strip;
        switch (chip_wire) {
            case 0:
                RoiRow = 0;
                break;
            case 1:
            case 2:
            case 3:
                RoiRow =
                    12 * (chip_wire - 1) + 2 * (hitId_wire - 1) + sub_wire + 1;
                break;
            default:
                break;
        }
    }

    roi = 4 * RoiRow + RoiColumn;

    return status;
}

///////////////////////////////////////////////////////////////
// HighPtID used in Simulation -> HighPtID in RDO
bool TgcCablingMap::getRDOHighPtIDfromSimHighPtID(const bool isForward,
                                                  const bool isStrip,
                                                  int& index, int& chip,
                                                  int& hitId) const {
    if (isForward) {
        if (isStrip) {  // FS
            index = 0;
            hitId--;
        } else if (!isStrip) {  // FW
            index = 0;
            hitId++;
        }
    } else {
        if (isStrip) {  // ES
            index = 0;
            hitId++;
        } else if (!isStrip) {  // EW
            if (index == 0 && chip == 0) {
                hitId = 1;
            } else if (index == 0 && chip == 1) {
                hitId++;
            } else if (index == 1 && chip == 0) {
                chip = 2;
                hitId++;
            } else if (index == 1 && chip == 1) {
                chip = 3;
                hitId++;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////
// HighPtID in RDO -> HighPtID used in Simulation
bool TgcCablingMap::getSimHighPtIDfromRDOHighPtID(const bool isForward,
                                                  const bool isStrip,
                                                  int& index, int& chip,
                                                  int& hitId) const {
    if (isForward) {
        if (isStrip) {  // FS
            index = 0;
            hitId++;
        } else if (!isStrip) {  // FW
            index = 0;
            hitId--;
        }
    } else {
        if (isStrip) {  // ES
            index = 0;
            hitId--;
        } else if (!isStrip) {  // EW
            switch (chip) {
                case 0:
                    index = 0;
                    hitId = 5;
                    break;
                case 1:
                    index = 0;
                    hitId--;
                    break;
                case 2:
                    index = 1;
                    chip = 0;
                    hitId--;
                    break;
                case 3:
                    index = 1;
                    chip = 1;
                    hitId--;
                    break;
                default:
                    break;
            }
        }
    }

    return true;
}

///////////////////////////////////////////////////////////////
// high pt coincidence IDs -> offline IDs
bool TgcCablingMap::getOfflineIDfromHighPtID(
    Identifier& offlineID, const int subDetectorID, const int rodID,
    const int sectorInReadout, const bool isStrip, const bool isForward,
    const int hpb, const int chip, const int hitID, const int pos) const {
    // all input is derived from TgcRawData
    int sswID = -1;
    int sbLoc = -1;
    int channelID = -1;

    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subDetectorID == m_AsideId) {
        sideType = TGCId::SideType::Aside;
    }
    if (subDetectorID == m_CsideId) {
        sideType = TGCId::SideType::Cside;
    }

    // SignalType, RegionType
    TGCId::SignalType signalType = (isStrip) ? TGCId::SignalType::Strip : TGCId::SignalType::Wire;
    TGCId::RegionType regionType = (isForward) ? TGCId::RegionType::Forward : TGCId::RegionType::Endcap;

    // ModuleType
    TGCId::ModuleType moduleType = TGCId::ModuleType::Undefined;
    if (signalType == TGCId::SignalType::Wire) {
        moduleType = TGCId::ModuleType::WD;
    } else {
        moduleType = TGCId::ModuleType::SD;
    }

    // Get ReadoutID for pivot plane
    bool status = getReadoutFromHighPtID(
        sideType, rodID, sswID, sbLoc, channelID, signalType, regionType,
        sectorInReadout, hpb, chip, hitID, pos, moduleType, false);

    if (!status) {
        return false;
    }

    // get OfflineID for pivot plane
    return getOfflineIDfromReadoutID(offlineID, subDetectorID, rodID, sswID,
                                     sbLoc, channelID);
}

///////////////////////////////////////////////////////////////
// offline IDs -> high pt coincidence IDs
bool TgcCablingMap::getHighPtIDfromOfflineID(const Identifier& offlineID,
                                             int& subDetectorID, int& rodID,
                                             int& sectorInReadout,
                                             bool& isStrip, bool& isForward,
                                             int& hpb, int& chip, int& hitID,
                                             int& pos) const {
    int sswID = -1;
    int sbLoc = -1;
    int channelID = -1;

    bool status = getReadoutIDfromOfflineID(offlineID, subDetectorID, rodID,
                                            sswID, sbLoc, channelID);
    if (!status) {
        return false;
    }

    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subDetectorID == m_AsideId) {
        sideType = TGCId::SideType::Aside;
    }
    if (subDetectorID == m_CsideId) {
        sideType = TGCId::SideType::Cside;
    }

    TGCId::SignalType signalType;
    TGCId::RegionType regionType;

    status = getHighPtIDFromReadout(sideType, rodID, sswID, sbLoc, channelID,
                                    signalType, regionType, sectorInReadout,
                                    hpb, chip, hitID, pos);
    if (!status) {
        return false;
    }

    isStrip = (signalType == TGCId::SignalType::Strip);
    isForward = (regionType == TGCId::RegionType::Forward);

    return true;
}

///////////////////////////////////////////////////////////////
// low pt coincidence IDs -> offline IDs
bool TgcCablingMap::getOfflineIDfromLowPtCoincidenceID(
    Identifier& offlineID, const int subDetectorID, const int rodID,
    const int sswID, const int sbLoc, const int block, const int pos,
    bool middle) const {
    int channelID = -1;

    // SideType
    TGCId::SideType sideType = TGCId::SideType::Undefined;
    if (subDetectorID == m_AsideId) {
        sideType = TGCId::SideType::Aside;
    }
    if (subDetectorID == m_CsideId) {
        sideType = TGCId::SideType::Cside;
    }

    bool status = getReadoutFromLowPtCoincidence(sideType, rodID, sswID, sbLoc,
                                                 channelID, block, pos, middle);
    if (!status) {
        return false;
    }

    return getOfflineIDfromReadoutID(offlineID, subDetectorID, rodID, sswID,
                                     sbLoc, channelID);
}

}  // namespace Muon
