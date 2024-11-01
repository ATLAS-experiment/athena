/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_IRTRESOLUTION_H
#define MUONCALIB_IRTRESOLUTION_H

#include "MdtCalibData/CalibFunc.h"

namespace MuonCalib {

    /** generic interface for a resolution function */

    class IRtResolution : public CalibFunc {
    public:
        using CalibFunc::CalibFunc;
        virtual ~IRtResolution() = default;
        virtual std::string typeName() const override { return "IRtResolution"; }

        /** returns resolution for a give time and background rate */
        virtual double resolution(double t, double bgRate = 0.0) const = 0;
    };

}  // namespace MuonCalib

#endif
