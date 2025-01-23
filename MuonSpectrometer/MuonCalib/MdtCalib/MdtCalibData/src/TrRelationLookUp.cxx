/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibData/TrRelationLookUp.h"

#include <cmath>
#include <algorithm>

namespace MuonCalib{
    unsigned TrRelationLookUp::nDoF() const {
        return m_times.size();
    }
    TrRelationLookUp::TrRelationLookUp(const IRtRelation& rtRelation) : ITrRelation{ParVec{}} {

        constexpr std::size_t numPoints(100);
        double stepSize{rtRelation.radius(rtRelation.tUpper()) / (numPoints - 1)};
        m_times.resize(numPoints);
        m_radii.resize(numPoints);

        for(std::size_t i = 0; i < numPoints; ++i){
            m_radii[i] = i * stepSize;
            m_times[i] = getTFromR(m_radii[i], rtRelation);
        }
        m_minRadius = m_radii.front();
        m_maxRadius = m_radii.back();
    }

    std::string TrRelationLookUp::name() const { return "TrRelationLookUp"; }

    std::optional<double> TrRelationLookUp::driftTime(const double r) const {
        if(r < minRadius()) return m_times.front();
        if(r > maxRadius()) return m_times.back();
        //Find the first element that is not less than r
        auto it = std::lower_bound(m_radii.begin(), m_radii.end(), r);
        //Find the distance to the element right before this one, since r lies between idx and idx+1
        std::size_t idx = std::distance(m_radii.begin(), it) - 1;
        double radius1 = m_radii[idx];
        double radius2 = m_radii[idx + 1];
        if (radius1 == radius2) return m_times[idx];
        double time1 = m_times[idx];
        double time2 = m_times[idx + 1];
        return time1 + (time2 - time1) * (r - radius1) / (radius2 - radius1);
    }

    std::optional<double> TrRelationLookUp::driftTimePrime(const double r) const {
        if(r < minRadius() || r > maxRadius()) return 0.;
        //do the same thing as above but return dt/dr
        auto it = std::lower_bound(m_radii.begin(), m_radii.end(), r);
        std::size_t idx = std::distance(m_radii.begin(), it) - 1;
        double radius1 = m_radii[idx];
        double radius2 = m_radii[idx + 1];
        if (radius1 == radius2) return std::nullopt;
        double time1 = m_times[idx];
        double time2 = m_times[idx + 1];
        return (time2 - time1) / (radius2 - radius1);
    }

    std::optional<double> TrRelationLookUp::driftTime2Prime(const double r) const {
        if(r < minRadius() || r > maxRadius()) return std::nullopt;
        return 0.0;
    }

    double TrRelationLookUp::minRadius() const { return m_minRadius; }
    double TrRelationLookUp::maxRadius() const { return m_maxRadius; }

    double TrRelationLookUp::getTFromR(const double radius, const IRtRelation& rtRelation) const {
        double precision{0.001};
        double tMax{rtRelation.tUpper()};
        double tMin{rtRelation.tLower()};

        //Search for the drift time
        while (tMax - tMin > 0.1 and std::abs(rtRelation.radius(0.5 * (tMin + tMax)) - radius) > precision) {
            double midPoint = 0.5 * (tMin + tMax);
            if (rtRelation.radius(midPoint) > radius) {
                tMax = midPoint;
            } else {
                tMin = midPoint;
            }
        }
        return 0.5 * (tMin + tMax);
    }
}