/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCALIBDATA_ITrRelation_H
#define MDTCALIBDATA_ITrRelation_H

// MuonCalib //
#include <MdtCalibData/CalibFunc.h>
#include <MuonCalibMath/UtilFunc.h>
#include "GeoModelUtilities/TransientConstSharedPtr.h"

#include <optional>
namespace MuonCalib{
    class ITrRelation;
    using ITrRelationPtr = GeoModel::TransientConstSharedPtr<ITrRelation>;
    class ITrRelation: public CalibFunc {
        public:
            /** @brief Constructor taking the input r-t relation & the vector of parameters */
            ITrRelation(const ParVec& parameters):
                CalibFunc{parameters}{}
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
            /** @brief Returns the number of degrees of freedom of the tr relation */
            virtual unsigned nDoF() const = 0;
            /** @brief Maps the radius interval [minRadius;maxRadius] to [-1;1] where 
             *         the minimal radius is on the lower end */
            double getReducedR(const double r) const {
                return  mapToUnitInterval(r, minRadius(), maxRadius());
            }
            /** @brief Returns the derivative of the reduced radisu w.r.t r */
            double getReducedRPrime() const {
                return unitIntervalPrime(minRadius(), maxRadius());
            }
    };
}


#endif
