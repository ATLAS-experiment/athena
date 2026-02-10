/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLING_HH
#define MUONTGC_CABLING_TGCCABLING_HH

#include <map>
#include <mutex>
#include <string>

#include "CxxUtils/checker_macros.h"
#include "MuonTGC_Cabling/TGCChannelId.h"
#include "MuonTGC_Cabling/TGCModuleId.h"

namespace MuonTGC_Cabling {

class TGCCableASDToPP;
class TGCCableHPBToSL;
class TGCCableInASD;
class TGCCableInPP;
class TGCCableInSLB;
class TGCCablePPToSLB;
class TGCCableSLBToHPB;
class TGCCableSLBToSSW;
class TGCCableSSWToROD;
class TGCChannelASDOut;
class TGCChannelId;
class TGCModuleMap;
class TGCModuleSLB;

class TGCCabling {
   public:
    //
    TGCCabling(const TGCCabling&) = delete;
    //
    TGCCabling& operator=(const TGCCabling&) = delete;

    struct Config {
        std::string fileNameASDtoPP{"MuonTGC_Cabling_ASD2PP.db"};
        std::string fileNameInPP{"MuonTGC_Cabling_PP.db"};
        std::string fileNamePPtoSL{"MuonTGC_Cabling_PP2SL.db"};
        std::string fileNameSLBtoROD{"MuonTGC_Cabling_SLB2ROD.db"};
        std::string fileNameASDtoPPdiff{"ASD2PP_diff_12_ONL.db"};
    };
    // Constructor & Destructor
    TGCCabling(const Config& cfg);

    virtual ~TGCCabling();

    enum MAXMINREADOUTIDS {
        MAXRODID = 12,
        MINRODID = 1,
        MAXSRODID = 3,
        MINSRODID = 1,
        MAXSSWID = 9,
        MINSSWID = 0,
        MAXSBLOC = 31,
        MINSBLOC = 0,
        MINCHANNELID = 40,
        MAXCHANNELID = 199
    };

    // slbIn --> AsdOut
    std::unique_ptr<TGCChannelId> getASDOutChannel(
        const TGCChannelId& slb_in) const;

    /////////////////////////////////////////////////////
    // readout ID -> SLB Module
    const TGCModuleId* getSLBFromReadout(TGCId::SideType side, int rodId,
                                         int sswId, int sbLoc) const;

    // readoutID -> RxID
    int getRxIdFromReadout(TGCId::SideType side, int rodId, int sswId,
                           int sbLoc) const;

    // SSW ID/RX ID-> SLB Module
    std::unique_ptr<TGCModuleId> getSLBFromRxId(TGCId::SideType side, int rodId,
                                                int sswId, int rxId) const;

    // SLB Module -> readout ID
    bool getReadoutFromSLB(const TGCModuleSLB& slb, TGCId::SideType& side,
                           int& rodId, int& sswId, int& sbLoc) const;

    // readout channel -> chamber channel
    std::unique_ptr<TGCChannelId> getASDOutFromReadout(
        TGCId::SideType side, int rodId, int sswId, int sbLoc, int channel,
        bool orChannel = false) const;

    // chamber channel -> readout channel
    bool getReadoutFromASDOut(const TGCChannelASDOut& asdout,
                              TGCId::SideType& side, int& rodId, int& sswId,
                              int& sbLoc, int& channel,
                              bool orChannel = false) const;

    // readout channel -> coincidence channel
    bool getHighPtIDFromReadout(TGCId::SideType side, int rodId, int sswId,
                                int sbLoc, int channel,
                                TGCId::SignalType& signal,
                                TGCId::RegionType& region, int& sectorInReadout,
                                int& hpbId, int& block, int& hitId,
                                int& pos) const;

    // coincidence channel -> readout channel
    bool getReadoutFromHighPtID(TGCId::SideType side, int rodId, int& sswId,
                                int& sbLoc, int& channel,
                                TGCId::SignalType signal,
                                TGCId::RegionType region, int sectorInReadout,
                                int hpbId, int block, int hitId, int pos,
                                TGCId::ModuleType moduleType,
                                bool orChannel) const;

    // readout channel -> coincidence channel
    bool getLowPtCoincidenceFromReadout(TGCId::SideType side, int rodId,
                                        int sswId, int sbLoc, int channel,
                                        int& block, int& pos,
                                        bool middle = false) const;

    // coincidence channel -> readout channel
    bool getReadoutFromLowPtCoincidence(TGCId::SideType side, int rodId,
                                        int sswId, int sbLoc, int& channel,
                                        int block, int pos,
                                        bool middle = false) const;

    // channel connection
    std::unique_ptr<TGCChannelId> getChannel(const TGCChannelId& channelId,
                                             TGCChannelId::ChannelIdType type,
                                             bool orChannel = false) const;
    // module connection
    TGCModuleMap getModule(const TGCModuleId& moduleId,
                           TGCModuleId::ModuleIdType type) const;

   private:
    std::unique_ptr<TGCCableInASD> m_cableInASD{};
    std::unique_ptr<TGCCableASDToPP> m_cableASDToPP{};
    std::unique_ptr<TGCCableInPP> m_cableInPP{};
    std::unique_ptr<TGCCablePPToSLB> m_cablePPToSLB{};
    std::unique_ptr<TGCCableInSLB> m_cableInSLB{};
    std::unique_ptr<TGCCableSLBToHPB> m_cableSLBToHPB{};
    std::unique_ptr<TGCCableHPBToSL> m_cableHPBToSL{};
    std::unique_ptr<TGCCableSLBToSSW> m_cableSLBToSSW{};
    std::unique_ptr<TGCCableSSWToROD> m_cableSSWToROD{};

    // Protected by mutex.
    mutable std::map<int, std::unique_ptr<TGCModuleId>> m_slbModuleIdMap
        ATLAS_THREAD_SAFE;
    mutable std::mutex m_mutex;

    int getIndexFromReadoutWithoutChannel(const TGCId::SideType side,
                                          const int rodId, const int sswId,
                                          const int sbLoc) const;
};

}  // namespace MuonTGC_Cabling

#endif
