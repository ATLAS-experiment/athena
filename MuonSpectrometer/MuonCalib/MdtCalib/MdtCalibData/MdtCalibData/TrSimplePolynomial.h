/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_TRSIMPLEPOLYNOMIAL_H
#define MUONCALIB_TRSIMPLEPOLYNOMIAL_H

#include <MdtCalibData/ITrRelation.h>

namespace MuonCalib{
    class TrSimplePolynomial: public ITrRelation {
        public:
            TrSimplePolynomial(const ParVec& vec);
            virtual std::string name() const override final;
            virtual std::optional<double> driftTime(const double r) const override final;
            virtual std::optional<double> driftTimePrime(const double r) const override final;
            virtual std::optional<double> driftTime2Prime(const double r) const override final;
            virtual double minRadius() const override final;
            virtual double maxRadius() const override final;
            virtual unsigned nDoF() const override final;

    };
}

#endif
