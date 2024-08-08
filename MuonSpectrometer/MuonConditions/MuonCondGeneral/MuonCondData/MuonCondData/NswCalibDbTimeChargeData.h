/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWCALIBDBTIMECHARGEDATA_H
#define MUONCONDDATA_NSWCALIBDBTIMECHARGEDATA_H

// STL includes
#include <vector>

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonCondData/Defs.h"


class NswCalibDbTimeChargeData {  

public:
    enum class CalibDataType{
        TDO,
        PDO        
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
    virtual ~NswCalibDbTimeChargeData() = default;

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
    // Copied from https://gitlab.cern.ch/atlas/athena/-/blob/master/MuonSpectrometer/MuonDetDescr/MuonReadoutGeometry/MuonReadoutGeometry/MuonDetectorManager.h
    enum sTgcGMRanges {
            NsTgStatEta = 6,      /// 3 x 2 sides (-3,-2,-1 and 1,2,3)
            NsTgStEtaOffset = 3,  /// needed offest to map (-3,-2,-1,1,2,3) to (0,1,2,3,4,5)
            NsTgStatPhi = 16,     // large and small sector together
            NsTgcStatLay = 4,   // 4 wedges of stgcs
            NsTgcChannelTypes =3, // Pads / Wires / Strips
            NsTgChamberLayer = 2
        };
    enum mmGMRanges {
            NMMcStatEta = 4,      /// 2 x 2 sides (-2,-1 and 1,2)
            NMMcStEtaOffset = 2,  /// needed offest to map (-2,-1,1,2) to (0,1,2,3)
            NMMcStatPhi = 16,     // large and small sector together
            NMMcStatLay = 4, /// 4 wedges of micromegas
            NMMcChamberLayer = 2
        };
    
    static constexpr int s_NumMaxSTgcElemets = NsTgStatEta * NsTgStatPhi * NsTgChamberLayer *NsTgcStatLay * NsTgcChannelTypes;
    static constexpr int s_NumMaxMMElements = NMMcStatEta * NMMcStatPhi * NMMcChamberLayer *NMMcStatLay;

	// containers
    struct CalibModule{
        std::vector<std::unique_ptr<CalibConstants>> channels{};
        Identifier layer_id{0};
    };
    using ChannelCalibMap = std::array<CalibModule, s_NumMaxSTgcElemets + s_NumMaxMMElements>;
    ChannelCalibMap m_pdo_data{};
    ChannelCalibMap m_tdo_data{};

    using ZeroCalibMap = std::map<CalibDataType, CalibConstants>;
    std::map<MuonCond::CalibTechType, ZeroCalibMap> m_zero{};

	// ID helpers
    const Muon::IMuonIdHelperSvc* m_idHelperSvc{};
};

std::ostream& operator<<(std::ostream& ostr, const NswCalibDbTimeChargeData::CalibConstants& obj);

CLASS_DEF( NswCalibDbTimeChargeData , 120842040 , 1 )
CLASS_DEF( CondCont<NswCalibDbTimeChargeData> , 217895024 , 1 )

#endif
