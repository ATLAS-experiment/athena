/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_RTRELATIONLOOKUP_H
#define MUONCALIB_RTRELATIONLOOKUP_H

#include <climits>
#include <iostream>


#include "MdtCalibData/IRtRelation.h"

namespace MuonCalib {

    /** Equidistant look up table for rt-relations with the time as key.
        The first parameter should be the time corresponding to the first bin.
        The second parameter should be the binsize.

        The r value is calculated by linear interpolation.
    */

    class RtRelationLookUp : public IRtRelation {
    public:
        explicit RtRelationLookUp(const ParVec &vec);

        virtual std::string name() const override final { return "RtRelationLookUp"; }

        /** returns drift radius for a given time */
        virtual double radius(double t) const override final;

        /** returns drift velocity for a given time */
        virtual double driftVelocity(double t) const override final;
        /** returns the acceleration for a given time */
        virtual double driftAcceleration(double t) const override final;

        /** return rt range */
        virtual double tLower() const override final;
        virtual double tUpper() const override final;
        virtual double tBinWidth() const override final;
        virtual unsigned nDoF() const override final; 

    private:
        int getBin(double t) const;

        // take offset due to m_t_min and binsize into account
        double getRadius(int bin) const { return par(bin + 2); }
        // returns best matching bin within rtRange
        int binInRtRange(double t) const;

        double m_t_min{0.};
        double m_bin_size{0.};
    };



}  // namespace MuonCalib
#endif
