/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_IRTRESOLUTION_H
#define MUONCALIB_IRTRESOLUTION_H

#include "MdtCalibData/CalibFunc.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"


namespace MuonCalib {
    class IRtResolution;
    using IRtResolutionPtr = GeoModel::TransientConstSharedPtr<IRtResolution>;
    /** @brief Generic interface to retrieve the resolution on the drift radius
     *         as a function of the drift time. The main method of the resolution function is the
     * 
     *          resolution(const double t, const double bgRate ) 
     *        method which returns for a given drift time the corresponding uncertainty on the drift radius
     */
    class IRtResolution : public CalibFunc {
      public:
        using CalibFunc::CalibFunc;
        virtual ~IRtResolution() = default;
        virtual std::string typeName() const override final { return "IRtResolution"; }
        /** Returns the number of degrees of freedom of the relation function  */
        virtual unsigned nDoF() const = 0;   
        /** returns resolution for a give time and background rate */
        virtual double resolution(double t, double bgRate = 0.0) const = 0;
    };

}  // namespace MuonCalib

#endif
