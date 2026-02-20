/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
    MuonTGC_CablingSvc.cxx
    Description : online-offline ID mapper for TGC
***************************************************************************/

#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"

#include <cmath>
#include <filesystem>
#include <fstream>

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonTGC_Cabling/TGCChannelASDIn.h"
#include "MuonTGC_Cabling/TGCChannelASDOut.h"
#include "MuonTGC_Cabling/TGCModuleMap.h"
#include "MuonTGC_Cabling/TGCModuleSLB.h"
#include "PathResolver/PathResolver.h"

using namespace MuonTGC_Cabling;

///////////////////////////////////////////////////////////////
void MuonTGC_CablingSvc::getReadoutIDRanges(int& maxRodId, int& maxSRodId,
                                            int& maxSswId, int& maxSbloc,
                                            int& minChannelId,
                                            int& maxChannelId) const {
    m_cabling->getReadoutIDRanges(maxRodId, maxSRodId, maxSswId, maxSbloc,
                                  minChannelId, maxChannelId);
}

///////////////////////////////////////////////////////////////
StatusCode MuonTGC_CablingSvc::initialize() {
    ATH_MSG_INFO("for 1/12 sector initialize");

    ATH_CHECK(m_idHelperSvc.retrieve());

    auto findCalibFile = [this](const std::string& db,
                                std::string& fileName) -> StatusCode {
        fileName = PathResolver::find_file(db, "DATAPATH");
        if (fileName.empty() || !std::filesystem::exists(fileName)) {
            ATH_MSG_ERROR("Cannot resolve database '" << db << "'");
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    };

    Muon::TgcCablingMap::Config cfg{};
    cfg.idHelperSvc = m_idHelperSvc.get();
    cfg.AsideId = m_AsideId;
    cfg.CsideId = m_CsideId;

    ATH_CHECK(findCalibFile(m_databaseASDToPP, cfg.fileNameASDtoPP));
    ATH_CHECK(findCalibFile(m_databaseInPP, cfg.fileNameInPP));
    ATH_CHECK(findCalibFile(m_databasePPToSL, cfg.fileNamePPtoSL));
    ATH_CHECK(findCalibFile(m_databaseSLBToROD, cfg.fileNameSLBtoROD));
    ATH_CHECK(findCalibFile(m_databaseASDToPP, cfg.fileNameASDtoPPdiff));

    // instantiate TGC cabling manager
    m_cabling = std::make_unique<Muon::TgcCablingMap>(cfg);

    return StatusCode::SUCCESS;
}

///////////////////////////////////////////////////////////////
bool MuonTGC_CablingSvc::getCoveragefromSRodID(
    const int srodID, int& startEndcapSector, int& coverageOfEndcapSector,
    int& startForwardSector, int& coverageOfForwardSector) const {

    return m_cabling->getCoveragefromSRodID(
        srodID, startEndcapSector, coverageOfEndcapSector, startForwardSector,
        coverageOfForwardSector);
}

///////////////////////////////////////////////////////////////
// Readout ID is ored
bool MuonTGC_CablingSvc::isOredChannel(const int subDetectorID, const int rodID,
                                       const int sswID, const int sbLoc,
                                       const int channelID) const {
    return m_cabling->isOredChannel(subDetectorID, rodID, sswID, sbLoc,
                                    channelID);
}

///////////////////////////////////////////////////////////////
// Offline ID has adjacent Readout ID
bool MuonTGC_CablingSvc::hasAdjacentChannel(const Identifier& offlineID) const {
    return m_cabling->hasAdjacentChannel(offlineID);
}

///////////////////////////////////////////////////////////////
// readout IDs -> offline IDs
bool MuonTGC_CablingSvc::getOfflineIDfromReadoutID(
    Identifier& offlineID, const int subDetectorID, const int rodID,
    const int sswID, const int sbLoc, const int channelID,
    bool orChannel) const {

    return m_cabling->getOfflineIDfromReadoutID(
        offlineID, subDetectorID, rodID, sswID, sbLoc, channelID, orChannel);
}

///////////////////////////////////////////////////////////////
// offline IDs -> readout IDs
bool MuonTGC_CablingSvc::getReadoutIDfromOfflineID(const Identifier& offlineID,
                                                   int& subDetectorID,
                                                   int& rodID, int& sswID,
                                                   int& sbLoc, int& channelID,
                                                   bool adChannel) const {
    return m_cabling->getReadoutIDfromOfflineID(
        offlineID, subDetectorID, rodID, sswID, sbLoc, channelID, adChannel);
}

///////////////////////////////////////////////////////////////
// offline ID -> online IDs
bool MuonTGC_CablingSvc::getOnlineIDfromOfflineID(
    const Identifier& offlineId, int& subSystemNumber, int& octantNumber,
    int& moduleNumber, int& layerNumber, int& rNumber, int& wireOrStrip,
    int& channelNumber) const {

    return m_cabling->getOnlineIDfromOfflineID(
        offlineId, subSystemNumber, octantNumber, moduleNumber, layerNumber,
        rNumber, wireOrStrip, channelNumber);
}

///////////////////////////////////////////////////////////////
// online IDs -> offline ID
bool MuonTGC_CablingSvc::getOfflineIDfromOnlineID(
    Identifier& offlineId, const int subSystemNumber, const int octantNumber,
    const int moduleNumber, const int layerNumber, const int rNumber,
    const int wireOrStrip, const int channelNumber) const {

    return m_cabling->getOfflineIDfromOnlineID(
        offlineId, subSystemNumber, octantNumber, moduleNumber, layerNumber,
        rNumber, wireOrStrip, channelNumber);
}
///////////////////////////////////////////////////////////////
// readout IDs -> online IDs
bool MuonTGC_CablingSvc::getOnlineIDfromReadoutID(
    const int subDetectorID, const int rodID, const int sswID, const int sbLoc,
    const int channelID, int& subsystemNumber, int& octantNumber,
    int& moduleNumber, int& layerNumber, int& rNumber, int& wireOrStrip,
    int& channelNumber, bool orChannel) const {

    return m_cabling->getOnlineIDfromReadoutID(
        subDetectorID, rodID, sswID, sbLoc, channelID, subsystemNumber,
        octantNumber, moduleNumber, layerNumber, rNumber, wireOrStrip,
        channelNumber, orChannel);
}

//////////////////////////////////////////////////////////
// online IDs -> readout IDs
bool MuonTGC_CablingSvc::getReadoutIDfromOnlineID(
    int& subDetectorID, int& rodID, int& sswID, int& sbLoc, int& channelID,
    const int subsystemNumber, const int octantNumber, const int moduleNumber,
    const int layerNumber, const int rNumber, const int wireOrStrip,
    const int channelNumber, bool adChannel) const {

    return m_cabling->getReadoutIDfromOnlineID(
        subDetectorID, rodID, sswID, sbLoc, channelID, subsystemNumber,
        octantNumber, moduleNumber, layerNumber, rNumber, wireOrStrip,
        channelNumber, adChannel);
}

// element ID -> readout IDs
bool MuonTGC_CablingSvc::getReadoutIDfromElementID(const Identifier& elementID,
                                                   int& subdetectorID,
                                                   int& rodID) const {

    return m_cabling->getReadoutIDfromElementID(elementID, subdetectorID,
                                                rodID);
}

///////////////////////////////////////////////////////////////
// readout IDs -> element ID
bool MuonTGC_CablingSvc::getElementIDfromReadoutID(
    Identifier& elementID, const int subDetectorID, const int rodID,
    const int sswID, const int sbLoc, const int channelID,
    bool orChannel) const {

    return m_cabling->getElementIDfromReadoutID(
        elementID, subDetectorID, rodID, sswID, sbLoc, channelID, orChannel);
}

///////////////////////////////////////////////////////////////
// readout ID -> SLB ID
bool MuonTGC_CablingSvc::getSLBIDfromReadoutID(int& phi, bool& isAside,
                                               bool& isEndcap, int& moduleType,
                                               int& id, const int subsectorID,
                                               const int rodID, const int sswID,
                                               const int sbLoc) const {

    return m_cabling->getSLBIDfromReadoutID(phi, isAside, isEndcap, moduleType,
                                            id, subsectorID, rodID, sswID,
                                            sbLoc);
}

///////////////////////////////////////////////////////////////
// readout ID -> rxID
bool MuonTGC_CablingSvc::getSLBAddressfromReadoutID(int& slbAddr,
                                                    const int subsectorID,
                                                    const int rodID,
                                                    const int sswID,
                                                    const int sbLoc) const {

    return m_cabling->getSLBAddressfromReadoutID(slbAddr, subsectorID, rodID,
                                                 sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// ROD_ID / SSW_ID / RX_ID -> SLB ID
bool MuonTGC_CablingSvc::getSLBIDfromRxID(int& phi, bool& isAside,
                                          bool& isEndcap, int& moduleType,
                                          int& id, const int subsectorID,
                                          const int rodID, const int sswID,
                                          const int rxId) const {

    return m_cabling->getSLBIDfromRxID(phi, isAside, isEndcap, moduleType, id,
                                       subsectorID, rodID, sswID, rxId);
}

///////////////////////////////////////////////////////////////
// SLB ID -> readout ID
bool MuonTGC_CablingSvc::getReadoutIDfromSLBID(
    const int phi, const bool isAside, const bool isEndcap,
    const int moduleType, const int id, int& subsectorID, int& rodID,
    int& sswID, int& sbLoc) const {

    return m_cabling->getReadoutIDfromSLBID(phi, isAside, isEndcap, moduleType,
                                            id, subsectorID, rodID, sswID,
                                            sbLoc);
}

///////////////////////////////////////////////////////////////
// readout ID -> SL ID
bool MuonTGC_CablingSvc::getSLIDfromReadoutID(int& phi, bool& isAside,
                                              bool& isEndcap,
                                              const int subsectorID,
                                              const int rodID, const int sswID,
                                              const int sbLoc) const {
    return m_cabling->getSLIDfromReadoutID(phi, isAside, isEndcap, subsectorID,
                                           rodID, sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// readout ID (only SROD) -> SL ID
bool MuonTGC_CablingSvc::getSLIDfromSReadoutID(int& phi, bool& isAside,
                                               const int subsectorID,
                                               const int srodID,
                                               const int sector,
                                               const bool forward) const {
    return m_cabling->getSLIDfromSReadoutID(phi, isAside, subsectorID, srodID,
                                            sector, forward);
}

///////////////////////////////////////////////////////////////
// SL ID -> readout ID
bool MuonTGC_CablingSvc::getReadoutIDfromSLID(const int phi, const bool isAside,
                                              const bool isEndcap,
                                              int& subsectorID, int& rodID,
                                              int& sswID, int& sbLoc) const {
    return m_cabling->getReadoutIDfromSLID(phi, isAside, isEndcap, subsectorID,
                                           rodID, sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// SL ID -> readout ID
bool MuonTGC_CablingSvc::getSReadoutIDfromSLID(const int phi,
                                               const bool isAside,
                                               const bool isEndcap,
                                               int& subsectorID, int& srodID,
                                               int& sswID, int& sbLoc) const {
    return m_cabling->getSReadoutIDfromSLID(phi, isAside, isEndcap, subsectorID,
                                            srodID, sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// HPT ID -> readout ID
bool MuonTGC_CablingSvc::getReadoutIDfromHPTID(
    const int phi, const bool isAside, const bool isEndcap, const bool,
    const int, int& subsectorID, int& rodID, int& sswID, int& sbLoc) const {
    return m_cabling->getReadoutIDfromSLID(phi, isAside, isEndcap, subsectorID,
                                           rodID, sswID, sbLoc);
}

///////////////////////////////////////////////////////////////
// CAUTION!!: return RDO value (not the value for simulation)
bool MuonTGC_CablingSvc::getHighPtIDfromROINumber(int roi, bool isForward,
                                                  bool isStrip, int& hpb,
                                                  int& chip, int& hitId,
                                                  int& sub) const {
    return m_cabling->getHighPtIDfromROINumber(roi, isForward, isStrip, hpb,
                                               chip, hitId, sub);
}

///////////////////////////////////////////////////////////////
bool MuonTGC_CablingSvc::getROINumberfromHighPtID(
    int& roi, bool isForward, int hpb_wire, int chip_wire, int hitId_wire,
    int sub_wire, int chip_strip, int hitId_strip, int sub_strip) const {

    return m_cabling->getROINumberfromHighPtID(
        roi, isForward, hpb_wire, chip_wire, hitId_wire, sub_wire, chip_strip,
        hitId_strip, sub_strip);
}

///////////////////////////////////////////////////////////////
// HighPtID used in Simulation -> HighPtID in RDO
bool MuonTGC_CablingSvc::getRDOHighPtIDfromSimHighPtID(const bool isForward,
                                                       const bool isStrip,
                                                       int& index, int& chip,
                                                       int& hitId) const {
    return m_cabling->getRDOHighPtIDfromSimHighPtID(isForward, isStrip, index,
                                                    chip, hitId);
}

///////////////////////////////////////////////////////////////
// HighPtID in RDO -> HighPtID used in Simulation
bool MuonTGC_CablingSvc::getSimHighPtIDfromRDOHighPtID(const bool isForward,
                                                       const bool isStrip,
                                                       int& index, int& chip,
                                                       int& hitId) const {

    return m_cabling->getSimHighPtIDfromRDOHighPtID(isForward, isStrip, index,
                                                    chip, hitId);
}

///////////////////////////////////////////////////////////////
// high pt coincidence IDs -> offline IDs
bool MuonTGC_CablingSvc::getOfflineIDfromHighPtID(
    Identifier& offlineID, const int subDetectorID, const int rodID,
    const int sectorInReadout, const bool isStrip, const bool isForward,
    const int hpb, const int chip, const int hitID, const int pos) const {

    return m_cabling->getOfflineIDfromHighPtID(
        offlineID, subDetectorID, rodID, sectorInReadout, isStrip, isForward,
        hpb, chip, hitID, pos);
}

///////////////////////////////////////////////////////////////
// offline IDs -> high pt coincidence IDs
bool MuonTGC_CablingSvc::getHighPtIDfromOfflineID(
    const Identifier& offlineID, int& subDetectorID, int& rodID,
    int& sectorInReadout, bool& isStrip, bool& isForward, int& hpb, int& chip,
    int& hitID, int& pos) const {
    return m_cabling->getHighPtIDfromOfflineID(
        offlineID, subDetectorID, rodID, sectorInReadout, isStrip, isForward,
        hpb, chip, hitID, pos);
}

///////////////////////////////////////////////////////////////
// low pt coincidence IDs -> offline IDs
bool MuonTGC_CablingSvc::getOfflineIDfromLowPtCoincidenceID(
    Identifier& offlineID, const int subDetectorID, const int rodID,
    const int sswID, const int sbLoc, const int block, const int pos,
    bool middle) const {

    return m_cabling->getOfflineIDfromLowPtCoincidenceID(
        offlineID, subDetectorID, rodID, sswID, sbLoc, block, pos, middle);
}
