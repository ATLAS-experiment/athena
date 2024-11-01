/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIB_IRTRELATION_H
#define MUONCALIB_IRTRELATION_H

#include "MdtCalibData/CalibFunc.h"
#include <optional>

namespace MuonCalib {

    /** generic interface for a rt-relation */

    class IRtRelation : public CalibFunc {
    public:
        using CalibFunc::CalibFunc;
        virtual ~IRtRelation() = default;
        virtual std::string typeName() const override final { return "IRtRelation"; }

        /** returns drift radius for a given time */
        virtual double radius(double t) const = 0;
        /** Returns the drift velocity for a given time */
        virtual double driftVelocity(double t) const = 0;
        /** Returns the acceleration of the r-t relation */
        virtual double driftAcceleration(double t) const = 0;
        /** Returns the lower time covered by the r-t */
        virtual double tLower() const = 0;
        /** Returns the upper time covered by the r-t */
        virtual double tUpper() const = 0;
        /** Returns the step-size for the sampling */
        virtual double tBinWidth() const =0;
        /** return the difference in total dirft time between the two multilayers (ML1 - ML2) */
        double GetTmaxDiff() const { return m_tmax_diff.value_or(0.); }

        bool hasTmaxDiff() const { return m_tmax_diff.has_value(); }

        /** set the difference in total drift time betwene the two multilayers (ML1 - ML2) */
        void SetTmaxDiff(const double d) { m_tmax_diff = d; }
      private:
        std::optional<double> m_tmax_diff{std::nullopt};
      protected:
        static constexpr double s_tBinWidth = 1.e-3;
    };

}  // namespace MuonCalib

#endif
