/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBRATION_RTRESOLUTIONSIMPLE_H
#define MDTCALIBRATION_RTRESOLUTIONSIMPLE_H

#include "MdtCalibData/IRtResolution.h"

namespace MuonCalibR4 {
    class RtResolutionSqrt : public MuonCalib::IRtResolution {
        public:
            explicit RtResolutionSqrt(ParVec& vec) : MuonCalib::IRtResolution(vec) {}
            std::string name() const;
            double resolution(double t, double bgRate = 0.0) const;

    };
}


#endif
