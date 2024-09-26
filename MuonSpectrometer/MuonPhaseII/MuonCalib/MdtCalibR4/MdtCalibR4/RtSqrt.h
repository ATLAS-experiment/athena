/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBRATION_RTSqrt_H
#define MDTCALIBRATION_RTSqrt_H

#include "MdtCalibData/IRtRelation.h"

namespace MuonCalibR4 {

    class RtSqrt : public MuonCalib::IRtRelation {
        public:
            explicit RtSqrt(ParVec& vec) : MuonCalib::IRtRelation(vec) {}

            std::string name() const;
            double radius(double t) const;
            double drdt(double t) const;
            double driftvelocity(double t) const;
            double tLower() const;
            double tUpper() const;

    };

}

#endif
