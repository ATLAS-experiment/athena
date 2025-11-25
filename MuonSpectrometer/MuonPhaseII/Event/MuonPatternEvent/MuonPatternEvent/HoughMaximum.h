/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNITION_HOUGHMAXIMUM__H
#define MUONR4_MUONPATTERNRECOGNITION_HOUGHMAXIMUM__H
#include <vector>

#include "MuonSpacePoint/SpacePointContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"

namespace MuonR4 {
/// @brief Data class to represent an eta maximum in hough space.
class HoughMaximum {
   public:
    using HitType = const SpacePoint*;
    /// @brief constructor.
    /// @param tanBeta: angle coordinate
    /// @param interceptY: intercept coordinate 
    /// @param counts: Weighted hit count of the maximum
    /// @param hits: list of measurements assigned to the maximum (owned by SG).
    /// @param bucket: Space point bucket out of which the hough maximum is built
    HoughMaximum(double tanBeta, double interceptY, double counts,
                 std::vector<HitType>&& hits,
                 const SpacePointBucket* bucket):
        m_tanBeta{tanBeta}, 
        m_interceptY{interceptY}, 
        m_counts{counts}, 
        m_hitsInMax{hits},
        m_bucket{bucket} {}
    /// @brief default c-tor, creates empty maximum with zero counts in the
    /// origin
    HoughMaximum() = default;
    /// @brief getter
    /// @return  the angular coordinate of the eta transform
    double tanBeta() const { return m_tanBeta; }

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
    /// @brief getter
    /// @return The parent space point bucket
    const SpacePointBucket* parentBucket() const {
        return m_bucket;
    }
    /// @brief getter
    /// @brief Return the associated chamber to the bucket
    const MuonGMR4::SpectrometerSector* msSector() const {
        return m_bucket->msSector();
    }

   private:
    double m_tanBeta{0.};                    // first coordinate
    double m_interceptY{0.};                  // second coordinate
    double m_counts{0.};                      // weighted counts
    std::vector<HitType> m_hitsInMax{};  // list of hits on maximum
    const SpacePointBucket* m_bucket{nullptr};
};
}  // namespace MuonR4

#endif
