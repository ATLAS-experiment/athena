/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBDATA_ITrRelation_H
#define MDTCALIBDATA_ITrRelation_H

// MuonCalib //
#include "MdtCalibData/IRtRelation.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

namespace MuonCalib{
    class ITrRelation: public CalibFunc {
        public:
            using IRtRelationPtr = GeoModel::TransientConstSharedPtr<IRtRelation>;
            /** @brief Constructor taking the input r-t relation & the vector of parameters */
            ITrRelation(const IRtRelationPtr& rtRelation, const ParVec& parameters):
                CalibFunc{parameters}, m_rt{rtRelation}{}
            /** @brief Desctructor */
            virtual ~ITrRelation() = default;
            virtual std::string typeName() const override final { return "ITrRelation"; }
            /** @brief Interface method for fetching the drift-time from the radius
             *         Returns a nullopt if the time is out of the boundaries */
            virtual std::optional<double> driftTime(const double r)const =0;
            /*** @brief Interface method for fetching the first derivative of the drift-time from the
             *          radius. Returns a nullopt if the time is out of the boundaries */
            virtual std::optional<double> driftTimePrime(const double r) const =0;
            /*** @brief Interface method for fetching the second derivative of the drift-time w.r.rt. the drift radius
             *          Returns a nullopt if the parsed time is ouf of the boundaries */
            virtual std::optional<double> driftTime2Prime(const double r) const = 0;
            /** @brief Returns the minimum drift-radius */
            virtual double minRadius() const = 0;
            /** @brief Returns the maximum drift-radius */
            virtual double maxRadius() const = 0;
        private:
            IRtRelationPtr m_rt{};

    };
}


#endif
