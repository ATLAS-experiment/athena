/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__HOUGHMAXIMUM__H
#define MUONR4__HOUGHMAXIMUM__H
#include <vector>

#include "MuonSpacePoint/MuonSpacePointContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

namespace MuonR4 {
/// @brief Data class to represent an eta maximum in hough space.
/// @tparam HitType: Data type encoding the hits used in the transform
template <class HitType>
class HoughMaximum_impl {
   public:
    /// @brief constructor.
    /// @param tanTheta: angle coordinate
    /// @param interceptY: intercept coordinate 
    /// @param counts: Weighted hit count of the maximum
    /// @param hits: list of measurements assigned to the maximum (owned by SG).
    HoughMaximum_impl(double tanTheta, double interceptY, double counts,
                 std::vector<HitType>&& hits)
        : m_tanTheta(tanTheta), m_interceptY(interceptY), m_counts(counts), m_hitsInMax(hits) {}
    /// @brief default c-tor, creates empty maximum with zero counts in the
    /// origin
    HoughMaximum_impl() = default;
    /// @brief getter
    /// @return  the angular coordinate of the eta transform
    double tanTheta() const { return m_tanTheta; }

    /// @brief getter
    /// @return  the intercept coordinate of the eta transform
    double interceptY() const { return m_interceptY; }
    /// @brief getter
    /// @return  the counts for this maximum
    double getCounts() const { return m_counts; }
    /// @brief getter
    /// @return  the hits assigned to this maximum
    const std::vector<HitType>& getHitsInMax() const {
        return m_hitsInMax;
    }

   private:
    double m_tanTheta{0.};                    // first coordinate
    double m_interceptY{0.};                  // second coordinate
    double m_counts{0.};                      // weighted counts
    std::vector<HitType> m_hitsInMax{};  // list of hits on maximum
};
}  // namespace MuonR4

#endif
