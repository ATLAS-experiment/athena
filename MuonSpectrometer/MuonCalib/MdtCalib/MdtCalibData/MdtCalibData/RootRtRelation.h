/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_ROOTRTRELATION_H
#define MUONCALIB_ROOTRTRELATION_H

#include <MdtCalibData/IRtRelation.h>

namespace MuonCalib{
    class RootRtRelation: public IRtRelation {
        public:
            RootRtRelation(const ParVec& pars);

            virtual ~RootRtRelation() = default;
        
            /** returns drift radius for a given time */
            virtual double radius(double t) const override final;
            /** Returns the drift velocity for a given time */
            virtual double driftVelocity(double t) const override final;
            /** Returns the acceleration of the r-t relation */
            virtual double driftAcceleration(double t) const override final;

            virtual std::string name() const override final;

            virtual double tLower() const override final;
            virtual double tUpper() const override final;        
    };
}

#endif