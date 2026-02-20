/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWCALIBDBTIMECHARGEDATA_H
#define MUONCONDDATA_NSWCALIBDBTIMECHARGEDATA_H



// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "AthenaBaseComps/AthMessaging.h"
#include "MuonCondData/Defs.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

// STL includes
#include <vector>
#include <map>
#include <iosfwd>



class NswCalibDbTimeChargeData: public AthMessaging {  

public:
    enum class CalibDataType{
        TDO,
        PDO,
        nTypes        
    };
    /// Helper struct to cache all calibration constants 
    /// in a common place of the memory
    struct CalibConstants{
       float slope{0.};
       float intercept{0.};
       //float slopeError{0.}; // keep for later
       //float interceptError{0.};       
    };
    
    NswCalibDbTimeChargeData(const Muon::IMuonIdHelperSvc* idHelperSvc);
    ~NswCalibDbTimeChargeData() = default;

    // setting functions
    void setData(CalibDataType type, const Identifier& chnlId, CalibConstants constants);
    void setZero(CalibDataType type, MuonCond::CalibTechType tech,  CalibConstants constants);

    // retrieval functions
    
    //// Retrieves the list of all identifiers for which calibration channels are available
    std::vector<Identifier> getChannelIds(const CalibDataType type, const std::string& tech, const std::string& side) const;
    /// Retrieves the calibration constant for a particular readout channel. If there is no calibration constant available,
    /// then the zero calibChannel is returned.
    const CalibConstants* getCalibForChannel(const CalibDataType type, const Identifier& channelId) const; 
    /// Returns the dummy calibration constant for the given technology type
    const CalibConstants* getZeroCalibChannel(const CalibDataType type, const MuonCond::CalibTechType tech) const;
 
private:
    
    int identToModuleIdx(const Identifier& chan_id) const;
    // ID helpers
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
    /// Segmentation of the elements is per NSW gasGap. Each wedge has 4 gasgaps
    const size_t m_nMmElements{m_idHelperSvc->hasMM() ?  
                                4* (m_idHelperSvc->mmIdHelper().detectorElement_hash_max() + 1) : 0};
    /// Additionally reserve space for the 3 channel types
    const size_t m_nStgcElements{m_idHelperSvc->hasSTGC() ? 
                                  3*4 *(m_idHelperSvc->stgcIdHelper().detectorElement_hash_max() +1): 0};
  
    // containers
    struct CalibModule {
        std::vector<std::unique_ptr<CalibConstants>> channels{};
        Identifier layer_id{};
    };

    using ChannelCalibMap = std::vector<CalibModule>; 
    ChannelCalibMap m_pdo_data{};
    ChannelCalibMap m_tdo_data{};

    using ZeroCalibMap = std::array<CalibConstants, Muon::MuonStationIndex::toInt(CalibDataType::nTypes)>;
    std::array<ZeroCalibMap, Muon::MuonStationIndex::toInt(MuonCond::CalibTechType::nTypes)> m_zero{};

};

std::ostream& operator<<(std::ostream& ostr, const NswCalibDbTimeChargeData::CalibConstants& obj);

CLASS_DEF( NswCalibDbTimeChargeData , 120842040 , 1 );
CONDCONT_DEF( NswCalibDbTimeChargeData , 217895024 );

#endif
