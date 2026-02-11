/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
    MuonTGC_CablingSvc.h
    Description : online-offline ID mapper for TGC
***************************************************************************/

#ifndef MUONTGC_CABLING_MUONTGC_CABLINGSVC_H
#define MUONTGC_CABLING_MUONTGC_CABLINGSVC_H

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/Service.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"

class Identifier;

class MuonTGC_CablingSvc : public AthService {
   public:
    using AthService::AthService;

    virtual ~MuonTGC_CablingSvc() = default;

    virtual StatusCode initialize() override;

    const MuonTGC_Cabling::TGCCabling* getTGCCabling() const;

    // give max value of the ROD ID
    int getMaxRodId() { return MuonTGC_Cabling::TGCCabling::MAXRODID; }

    // give max value of ReadoutID parameters
    /** @todo Tobe ported */
    void getReadoutIDRanges(int& maxRodId, int& maxSRodId, int& maxSswId,
                            int& maxSbloc, int& minChannelId,
                            int& maxChannelId) const;

    bool getCoveragefromSRodID(const int srodID, int& startEndcapSector,
                               int& coverageOfEndcapSector,
                               int& startForwardSector,
                               int& coverageOfForwardSector) const;

    // Readout ID is ored
    /** @brief To be ported */
    bool isOredChannel(const int subDetectorID, const int rodID,
                       const int sswID, const int sbLoc,
                       const int channelID) const;

    // Offline ID has adjacent Readout ID
    bool hasAdjacentChannel(const Identifier& offlineID) const;
    // readout IDs -> offline IDs
    /** @brief To be ported */
    bool getOfflineIDfromReadoutID(Identifier& offlineID,
                                   const int subDetectorID, const int rodID,
                                   const int sswID, const int sbLoc,
                                   const int channelID,
                                   bool orChannel = false) const;

    // offline IDs -> readout IDs
    bool getReadoutIDfromOfflineID(const Identifier& offlineID,
                                   int& subDetectorID, int& rodID, int& sswID,
                                   int& sbLoc, int& channelID,
                                   bool adChannel = false) const;

    // offline ID -> online IDs
    bool getOnlineIDfromOfflineID(const Identifier& offlineID,
                                  int& subsystemNumber, int& octantNumber,
                                  int& moduleNumber, int& layerNumber,
                                  int& rNumber, int& wireOrStrip,
                                  int& channelNumber) const;

    // online IDs -> offline ID
    bool getOfflineIDfromOnlineID(Identifier& offlineID,
                                  const int subsystemNumber,
                                  const int octantNumber,
                                  const int moduleNumber, const int layerNumber,
                                  const int rNumber, const int wireOrStrip,
                                  const int channelNumber) const;

    // readout IDs -> online IDs
    bool getOnlineIDfromReadoutID(const int subDetectorID, const int rodID,
                                  const int sswID, const int sbLoc,
                                  const int channelID, int& subsystemNumber,
                                  int& octantNumber, int& moduleNumber,
                                  int& layerNumber, int& rNumber,
                                  int& wireOrStrip, int& channelNumber,
                                  bool orChannel = false) const;

    // online IDs -> readout IDs
    bool getReadoutIDfromOnlineID(int& subDetectorID, int& rodID, int& sswID,
                                  int& sbLoc, int& channelID,
                                  const int subsystemNumber,
                                  const int octantNumber,
                                  const int moduleNumber, const int layerNumber,
                                  const int rNumber, const int wireOrStrip,
                                  const int channelNumber,
                                  bool adChannel = false) const;

    // element ID -> readout IDs
    bool getReadoutIDfromElementID(const Identifier& elementID,
                                   int& subdetectorID, int& rodID) const;

    // readout IDs -> element ID
    bool getElementIDfromReadoutID(Identifier& elementID,
                                   const int subDetectorID, const int rodID,
                                   const int sswID, const int sbLoc,
                                   const int channelID,
                                   bool orChannel = false) const;

    // HPT ID -> readout ID
    bool getReadoutIDfromHPTID(const int phi, const bool isAside,
                               const bool isEndcap, const bool isStrip,
                               const int id, int& subsectorID, int& rodID,
                               int& sswID, int& sbLoc) const;

    // readout ID -> SLB ID
    bool getSLBIDfromReadoutID(int& phi, bool& isAside, bool& isEndcap,
                               int& moduleType, int& id, const int subsectorID,
                               const int rodID, const int sswID,
                               const int sbLoc) const;

    // readout ID -> slbAddr
    bool getSLBAddressfromReadoutID(int& slbAddr, const int subsectorID,
                                    const int rodID, const int sswID,
                                    const int sbLoc) const;

    // ROD_ID / SSW_ID / RX_ID -> SLB ID
    bool getSLBIDfromRxID(int& phi, bool& isAside, bool& isEndcap,
                          int& moduleType, int& id, const int subsectorID,
                          const int rodID, const int sswID,
                          const int rxId) const;

    // SLB ID -> readout ID
    bool getReadoutIDfromSLBID(const int phi, const bool isAside,
                               const bool isEndcap, const int moduleType,
                               const int id, int& subsectorID, int& rodID,
                               int& sswID, int& sbLoc) const;

    // readout ID (ROD) -> SL ID
    bool getSLIDfromReadoutID(int& phi, bool& isAside, bool& isEndcap,
                              const int subsectorID, const int rodID,
                              const int sswID, const int sbLoc) const;

    // readout ID (SROD) -> SL ID
    bool getSLIDfromSReadoutID(int& phi, bool& isAside, const int subsectorID,
                               const int srodID, const int sector,
                               const bool forward) const;
    // SL ID -> readout ID ( ROD )
    bool getReadoutIDfromSLID(const int phi, const bool isAside,
                              const bool isEndcap, int& subsectorID, int& rodID,
                              int& sswID, int& sbLoc) const;

    // SL ID -> readout ID ( SROD )
    bool getSReadoutIDfromSLID(const int phi, const bool isAside,
                               const bool isEndcap, int& subsectorID,
                               int& srodID, int& sswID, int& sbLoc) const;
    // HighPtID used in Simulation -> HighPtID in RDO
    bool getRDOHighPtIDfromSimHighPtID(const bool isForward, const bool isStrip,
                                       int& index, int& chip, int& hitId) const;

    // HighPtID in RDO -> HighPtID used in Simulation
    bool getSimHighPtIDfromRDOHighPtID(const bool isForward, const bool isStrip,
                                       int& index, int& chip, int& hitId) const;

    // high pt coincidence IDs -> offline IDs
    bool getOfflineIDfromHighPtID(Identifier& offlineID,
                                  const int subDetectorID, const int rodID,
                                  const int sectorInReadout, const bool isStrip,
                                  const bool isForward, const int hpb,
                                  const int chip, const int hitID,
                                  const int pos) const;

    // offline IDs -> high pt coincidence IDs
    bool getHighPtIDfromOfflineID(const Identifier& offlineID,
                                  int& subDetectorID, int& rodID,
                                  int& sectorInReadout, bool& isStrip,
                                  bool& isForward, int& hpb, int& chip,
                                  int& hitID, int& pos) const;

    // HPT HitID -> ROI Number
    bool getROINumberfromHighPtID(int& roi, bool isForward, int hpb_wire,
                                  int chip_wire, int hitId_wire, int sub_wire,
                                  int chip_strip, int hitId_strip,
                                  int sub_strip) const;

    // HPT HitID -> ROI Number
    bool getHighPtIDfromROINumber(int roi, bool isForward, bool isStrip,
                                  int& hpb, int& chip, int& hitID,
                                  int& sub) const;

    // low pt coincidence IDs -> offline IDs
    bool getOfflineIDfromLowPtCoincidenceID(Identifier& offlineID,
                                            const int subDetectorID,
                                            const int rodID, const int sswID,
                                            const int sbLoc, const int block,
                                            const int pos,
                                            bool middle = false) const;

   private:
    /////////////////////////////////////////////////////////////
    // channel connection
    std::unique_ptr<MuonTGC_Cabling::TGCChannelId> getChannel(
        const MuonTGC_Cabling::TGCChannelId& channelId,
        MuonTGC_Cabling::TGCChannelId::ChannelIdType type,
        bool orChannel = false) const;

    // module connection
    MuonTGC_Cabling::TGCModuleMap getModule(
        const MuonTGC_Cabling::TGCModuleId& moduleId,
        MuonTGC_Cabling::TGCModuleId::ModuleIdType type) const;

    ///////////////////////

    std::unique_ptr<Muon::TgcCablingMap> m_cabling;
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    IntegerProperty m_AsideId{this, "AsideId", 103};
    IntegerProperty m_CsideId{this, "CsideId", 104};

    StringProperty m_databaseASDToPP{this, "databaseASDToPP",
                                     "MuonTGC_Cabling_ASD2PP.db"};
    StringProperty m_databaseInPP{this, "databaseInPP",
                                  "MuonTGC_Cabling_PP.db"};
    StringProperty m_databasePPToSL{this, "databasePPToSL",
                                    "MuonTGC_Cabling_PP2SL.db"};
    StringProperty m_databaseSLBToROD{this, "databaseSLBToROD",
                                      "MuonTGC_Cabling_SLB2ROD.db"};
    StringProperty m_databaseASDToPPdiff{this, "databaseASDtoPPdiff",
                                         "ASD2PP_diff_12_OFL.db"};
};

inline const MuonTGC_Cabling::TGCCabling* MuonTGC_CablingSvc::getTGCCabling()
    const {
    return m_cabling.get();
}

#endif  // MUONTGC_CABLING_MUONTGC_CABLINGSVC_H
