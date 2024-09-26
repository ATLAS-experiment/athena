/*
  Copyright (C) 2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/IRtRelation.h"
#include "MdtCalibData/TrRelation.h"

namespace MuonCalibR4 {
    
    class TrSqrt : public MuonCalib::TrRelation {

        public:
            using TrRelation::TrRelation;

            TrSqrt(const MuonCalib::IRtRelation& rt) : TrRelation(rt) {}

            double tFromR(const double  r, bool& out_of_bound_flag) const;

            double rFromT(const double  t, bool& out_of_bound_flag) const;            
    };
}