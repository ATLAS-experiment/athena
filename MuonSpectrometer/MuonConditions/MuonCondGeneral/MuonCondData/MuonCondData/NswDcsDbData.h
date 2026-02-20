/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWDCSDBDATA_H
#define MUONCONDDATA_NSWDCSDBDATA_H

// STL includes
#include <set>
#include <vector>

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCondData/Defs.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"

class NswDcsDbData {  

public:
    enum class DcsTechType{
        MMG, // MM channels
        MMD, // MM drift channels
        STG  // sTGC
    };
    using DcsDataType = MuonCond::DcsDataType;
    using DcsFsmState = MuonCond::DcsFsmState;
    using DcsConstants = MuonCond::DcsConstants;
    

    struct TDaqConstants{
        uint64_t timeSince{0};
        uint64_t timeUntil{0};
        unsigned elink{0};
        bool permanentlyDisabled{false};
        bool operator<(const NswDcsDbData::TDaqConstants& other)const{
            if(elink != other.elink) {
                return elink < other.elink;
            }
            return timeUntil < other.timeSince;
        }
    };
    
    NswDcsDbData(const Muon::IMuonIdHelperSvc* idHelperSvc, const MuonGM::MuonDetectorManager* muonGeoMgr);
    virtual ~NswDcsDbData() = default;

    // setting functions
    void setDataHv(const DcsTechType tech, 
                   const Identifier& chnlId, 
                   DcsConstants constants);
    void setDataTDaq(const DcsTechType tech, 
                    const Identifier& chnlId, 
                    uint64_t timeSince, 
                    uint64_t timeUntil, 
                    unsigned int elink, 
                    bool permanentlyDisabled);
    void setDataEltx(const DcsTechType tech, 
                     const Identifier& chnlId);
    
    // retrieval functions
    
    //// Retrieves the list of all identifiers for which calibration channels are available
    std::vector<Identifier> getChannelIdsHv(const DcsTechType tech, const std::string& side) const;
    /// Retrieves the calibration constant for a particular readout channel.
    const DcsConstants* getDataForChannelHv(const DcsTechType tech, const Identifier& channelId, bool issTgcQ1OuterHv) const; 
    
    /// Returns whether the channel is alive, i.e. DCS state on, etc...
    bool isGood(const EventContext& ctx, const Identifier& channelId, bool issTgcQ1OuterHv = false) const;
    bool isGoodHv(const Identifier& channelId, bool issTgcQ1OuterHv = false) const;
    bool isGoodTDaq(const EventContext& ctx, const Identifier& channelId, bool &permanentlyDisabled) const;
    bool isGoodEltx(const Identifier& channelId) const;
    bool isConnectedChannel(const Identifier& channelId) const;

private:
    
    unsigned int identToModuleIdx(const Identifier& chan_id) const;
    
    // containers
    struct DcsModule{
        std::vector<std::unique_ptr<DcsConstants>> channels{};
        Identifier layer_id{};
    };
    using ChannelDcsMap = std::vector<DcsModule>;
    ChannelDcsMap m_data_hv_mmg{};
    ChannelDcsMap m_data_hv_mmd{};
    ChannelDcsMap m_data_hv_stg{};
    using ChannelTDaqMap = std::vector<std::map<Identifier, std::set<TDaqConstants>>>;
    ChannelTDaqMap m_data_tdaq_mmg{};
    ChannelTDaqMap m_data_tdaq_stg{};
    using ChannelEltxMap = std::vector<std::unordered_set<Identifier>>;
    ChannelEltxMap m_data_eltx_mmg{};
    ChannelEltxMap m_data_eltx_stg{};

    // ID helpers
     const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
    const MuonGM::MuonDetectorManager* m_muonGeoMgr{nullptr}; 

};

std::ostream& operator<<(std::ostream& ostr, const NswDcsDbData::TDaqConstants& obj);



CLASS_DEF( NswDcsDbData , 99551304 , 1 );
CONDCONT_MIXED_DEF( NswDcsDbData , 125092872);

#endif
