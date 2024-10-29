/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_POLYRTRELATION_H
#define MUONCALIB_POLYRTRELATION_H

#include <MdtCalibData/IRtRelation.h>

/** @brief Rt relation parameterized as a general polynomial */
namespace MuonCalib{
    class PolyRtRelation: public IRtRelation{
        public:
            PolyRtRelation(const ParVec& pars);
            virtual ~PolyRtRelation() = default;
        
            /** returns drift radius for a given time */
            virtual double radius(double t) const override final;
            /** Returns the drift velocity for a given time */
            virtual double driftVelocity(double t) const override final;
            /** Returns the acceleration of the r-t relation */
            virtual double driftAcceleration(double t) const override final;

            virtual double tLower() const override final;
            virtual double tUpper() const override final;

    };
    
}

#endif