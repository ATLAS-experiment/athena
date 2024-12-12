/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_TRCHEBYSHEV_H
#define MUONCALIB_TRCHEBYSHEV_H

#include <MdtCalibData/ITrRelation.h>


namespace MuonCalib{
    class TrChebyshev: public ITrRelation {
        public:
            TrChebyshev(const IRtRelationPtr& rtRelation, const ParVec& vec);
            virtual std::string name() const override final;
            virtual std::optional<double> driftTime(const double r) const override final;
            virtual std::optional<double> driftTimePrime(const double r) const override final;
            virtual std::optional<double> driftTime2Prime(const double r) const override final;
            virtual double minRadius() const override final;
            virtual double maxRadius() const override final;
        private:
            double m_minRadius{};
            double m_maxRadius{};
            std::size_t m_polynomialOrder{15};
    };
}



#endif