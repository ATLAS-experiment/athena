/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONFASTRECOEVENT_GLOBALPATTERN__H
#define MUONR4_MUONFASTRECOEVENT_GLOBALPATTERN__H

#include "MuonSpacePoint/SpacePoint.h"
#include "MuonSpacePoint/SpacePointContainer.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonTrackEvent/ExpandedSector.h"

namespace MuonR4 {
/// @brief Data class to represent an eta maximum in hough space.
class GlobalPattern {
   public:
    using HitType = const SpacePoint*;
    using StIndex = Muon::MuonStationIndex::StIndex;
    using HitCollection = std::unordered_map<StIndex, std::vector<HitType>>;
    
    /// @brief c-tor consuming the hit collection per station
    GlobalPattern(HitCollection&& hitPerStation, 
                  std::vector<const SpacePointBucket*>&& parentBuckets);
    GlobalPattern() = delete;
    /// @brief Copy c-tor
    GlobalPattern(const GlobalPattern& other) = default;

    /// @brief Set the average theta of the pattern
    void setTheta(double theta) { m_theta = theta; }
    /// @brief Set the average phi of the pattern
    void setPhi(double phi) { m_phi = phi; }
    /// @brief Set the main sector of the pattern    
    void setSector(std::int8_t sector) { m_sector = ExpandedSector{sector}; }
    /// @brief Set the number of precision layers in the pattern
    void setNPrecisionLayers(unsigned n) { m_nPrecisionLayers = n; }
    /// @brief Set the number of trigger layers in the pattern
    void setNTriggerLayers(unsigned n) { m_nTriggerLayers = n; }
    /// @brief Set the number of phi layers in the pattern
    void setNPhiLayers(unsigned n) { m_nPhiLayers = n; }
    /// @brief Set the mean over eta hits of the square of their residual divided by acceptance window from pattern finding
    void setMeanNormResidual2(double res) { m_meanNormResidual2 = res; }
    

    /// @brief Return the average global theta of the pattern
    double theta() const { return m_theta; }
    /// @brief Return the average global phi of the pattern
    double phi() const { return m_phi; }
    /// @brief Return the main sector where the pattern is located
    unsigned sector() const { return m_sector.msSector(); }
    /// @brief Return the associated sector to the bucket
    unsigned secondarySector() const { return m_sector.adjacentMsSector(); }
    /// @brief Return whether the pattern is located in the overlap region between two sectors
    bool isSectorOverlap() const { return sector() != secondarySector(); }
    /// @brief Return the expanded sector of the pattern
    const ExpandedSector& expSector() const { return m_sector; }
    /// @brief Return the sector phi of the pattern. It is the central phi of the sector or the the value at the edge in case of overlap
    double sectorPhi() const;
    /// @brief Return the associated stations to the pattern
    std::vector<StIndex> getStations() const;
    /// @brief Return the pattern hits in the given station
    const std::vector<HitType>& hitsInStation(StIndex station) const;
    /// @brief Return the parent buckets of the pattern in the given station
    const std::vector<const SpacePointBucket*> bucketsInStation(StIndex station) const;
    /// @brief Return all the parent buckets of the pattern
    const std::vector<const SpacePointBucket*>& getParentBuckets() const { return m_parentBuckets; }
    /// @brief Return the number of precision layers in the pattern
    unsigned nPrecisionLayers() const { return m_nPrecisionLayers; }
    /// @brief Return the number of trigger layers in the pattern
    unsigned nTriggerLayers() const { return m_nTriggerLayers; }
    /// @brief Return the number of phi layers in the pattern
    unsigned nPhiLayers() const { return m_nPhiLayers; }
    /// @brief Return the mean over eta hits of the square of their residual divided by acceptance window from pattern finding
    double meanNormResidual2() const { return m_meanNormResidual2; }
    /// @brief Return the hits per station
    const HitCollection& hitsPerStation() const { return m_hitsInStation; }

    /** @brief The print-out operator */
    friend std::ostream& operator<<(std::ostream& ostr, const GlobalPattern& gp) {
        gp.print(ostr);
        return ostr;
    }
    /** @brief Equality operator */
    bool operator==(const GlobalPattern& other) const {
        return m_hitsInStation == other.m_hitsInStation;
    }

   private:
    void print(std::ostream& ostr) const;

    /** average global theta of the pattern */
    double m_theta{0.};
    /** average global phi of the pattern */
    double m_phi{0.};
    /** Number of precision layers */
    unsigned m_nPrecisionLayers{0};
    /** Number of trigger layers */
    unsigned m_nTriggerLayers{0};
    /** Number of phi layers */
    unsigned m_nPhiLayers{0};
    /** Mean over eta hits of the square of their residual divided by acceptance window from pattern finding */
    double m_meanNormResidual2{0.};

    // The pattern can extend over two sectors in the overlap region
    ExpandedSector m_sector{static_cast<int8_t>(0)};

    /** Hits of the pattern organized per station */
    const HitCollection m_hitsInStation{};
    /** Collection of parent buckets */
    const std::vector<const SpacePointBucket*> m_parentBuckets{};
};
/** @brief Abrivation of the GlobalPattern container type */
using GlobalPatternContainer = DataVector<GlobalPattern>;
}  // namespace MuonR4

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( MuonR4::GlobalPatternContainer , 1197530715 , 1 )

#endif
