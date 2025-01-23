/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBDATA_TRRELATIONLOOKUP_H
#define MDTCALIBDATA_TRRELATIONLOOKUP_H

#include "MdtCalibData/ITrRelation.h"
#include "MdtCalibData/IRtRelation.h"

#include <vector>

namespace MuonCalib {
    class TrRelationLookUp: public ITrRelation {
        public:
            TrRelationLookUp(const IRtRelation& rtRelation);
            virtual std::string name() const override final;
            virtual std::optional<double> driftTime(const double r) const override final;
            virtual std::optional<double> driftTimePrime(const double r) const override final;
            virtual std::optional<double> driftTime2Prime(const double r) const override final;
            virtual double minRadius() const override final;
            virtual double maxRadius() const override final;
            virtual unsigned nDoF() const override final;
        private:
            std::vector<double> m_times{};
            std::vector<double> m_radii{};
            double m_minRadius{};
            double m_maxRadius{};
            double getTFromR(const double radius, const IRtRelation& rtRelation) const;
    };
}

#endif