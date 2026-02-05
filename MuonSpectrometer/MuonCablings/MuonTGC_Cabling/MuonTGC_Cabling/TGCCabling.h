/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CABLING_TGCCABLING_HH
#define MUONTGC_CABLING_TGCCABLING_HH

#include <map>
#include <mutex>
#include <string>

#include "CxxUtils/checker_macros.h"
#include "MuonTGC_Cabling/TGCChannelId.h"
#include "MuonTGC_Cabling/TGCModuleId.h"

class StatusCode;

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
   private:  // hide default constructor, copy constructor and assignment
    TGCCabling() = delete;
    TGCCabling(const TGCCabling&) = delete;
    TGCCabling& operator=(const TGCCabling&) = delete;

   public:
    // Constructor & Destructor
    TGCCabling(const std::string& filenameASDToPP,
               const std::string& filenameInPP,
               const std::string& filenamePPToSL,
               const std::string& filenameSLBToROD);

    virtual ~TGCCabling(void);

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
    virtual TGCChannelId* getASDOutChannel(const TGCChannelId* slb_in) const;

    /////////////////////////////////////////////////////
    // readout ID -> SLB Module
    const TGCModuleId* getSLBFromReadout(TGCId::SideType side, int rodId,
                                         int sswId, int sbLoc) const;

    // readoutID -> RxID
    int getRxIdFromReadout(TGCId::SideType side, int rodId, int sswId,
                           int sbLoc) const;

    // SSW ID/RX ID-> SLB Module
    TGCModuleId* getSLBFromRxId(TGCId::SideType side, int rodId, int sswId,
                                int rxId) const;

    // SLB Module -> readout ID
    bool getReadoutFromSLB(const TGCModuleSLB* slb, TGCId::SideType& side,
                           int& rodId, int& sswId, int& sbLoc) const;

    // readout channel -> chamber channel
    TGCChannelId* getASDOutFromReadout(TGCId::SideType side, int rodId,
                                       int sswId, int sbLoc, int channel,
                                       bool orChannel = false) const;

    // chamber channel -> readout channel
    bool getReadoutFromASDOut(const TGCChannelASDOut* asdout,
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
    TGCChannelId* getChannel(const TGCChannelId* channelId,
                             TGCChannelId::ChannelIdType type,
                             bool orChannel = false) const;
    // module connection
    TGCModuleMap* getModule(const TGCModuleId* moduleId,
                            TGCModuleId::ModuleIdType type) const;

   public:
    // readin database which describes difference from ASDToPP.db
    StatusCode updateCableASDToPP();

   private:
    TGCCableInASD* m_cableInASD;
    TGCCableASDToPP* m_cableASDToPP;
    TGCCableInPP* m_cableInPP;
    TGCCablePPToSLB* m_cablePPToSLB;
    TGCCableInSLB* m_cableInSLB;
    TGCCableSLBToHPB* m_cableSLBToHPB;
    TGCCableHPBToSL* m_cableHPBToSL;
    TGCCableSLBToSSW* m_cableSLBToSSW;
    TGCCableSSWToROD* m_cableSSWToROD;

    // Protected by mutex.
    mutable std::map<int, TGCModuleId*> m_slbModuleIdMap ATLAS_THREAD_SAFE;
    mutable std::mutex m_mutex;

    int getIndexFromReadoutWithoutChannel(const TGCId::SideType side,
                                          const int rodId, const int sswId,
                                          const int sbLoc) const;
};

}  // namespace MuonTGC_Cabling

#endif
