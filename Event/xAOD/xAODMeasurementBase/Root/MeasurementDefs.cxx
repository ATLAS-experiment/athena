
/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace xAOD{
    std::string toString(const UncalibMeasType type) {
        switch (type){
            using enum UncalibMeasType;
            case Other: return "Other";
            case PixelClusterType: return "PixelClusterType";
            case StripClusterType: return "StripClusterType";
            case MdtDriftCircleType: return "MdtDriftCircleType";
            case RpcStripType: return "RpcStripType";
            case TgcStripType: return "TgcStripType";
            case MMClusterType: return "MMClusterType";
            case sTgcStripType: return "sTgcStripType";
            case HGTDClusterType: return "HGTDClusterType";
            case nTypes: return "nTypes";
        }
        return "unknown";
    }
}