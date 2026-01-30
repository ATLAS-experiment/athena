/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCONDDATA_DEFS_H
#define MUONCONDDATA_DEFS_H

#include <string>
#include <iostream>

namespace MuonCond {
    enum class DcsDataType{
        HV,
        LV
    };
    enum class DcsFsmState{
        NONE,
        UNKNOWN,
        OFF,
        ON,
        STANDBY,
        DEAD,
        UNPLUGGED,
        RAMP_UP,
        RAMP_DOWN,
        TRIP,
        RECOVERY,
        LOCKED
    };
    /// Helper struct to cache all dcs constants 
    /// in a common place of the memory
    struct DcsConstants{
       float standbyVolt{0.f};
       float readyVolt{0.f};
       DcsFsmState fsmState{DcsFsmState::NONE};
    };

    enum class CalibTechType{
        MM,
        STGC,
        nTypes        
    };


    DcsFsmState getFsmStateEnum(const std::string& fsmState);
    std::string getFsmStateStrg(DcsFsmState fsmState); 
    
    std::ostream& operator<<(std::ostream& ostr, const DcsConstants& dcs);

}
#endif