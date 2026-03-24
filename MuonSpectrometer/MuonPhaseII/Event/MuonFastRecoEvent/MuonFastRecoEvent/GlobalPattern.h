/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONFASTRECOEVENT_GLOBALPATTERN__H
#define MUONR4_MUONFASTRECOEVENT_GLOBALPATTERN__H

#include "MuonSpacePoint/SpacePoint.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "AthContainers/DataVector.h"

namespace MuonR4 {
/// @brief Data class to represent an eta maximum in hough space.
class GlobalPattern {
   public:
    using HitType = const SpacePoint*;
    using StIndex = Muon::MuonStationIndex::StIndex;
    using HitCollection = std::unordered_map<StIndex, std::vector<HitType>>;
    
    /// @brief c-tor consuming the hit collection per station
    GlobalPattern(HitCollection&& hitPerStation);
    GlobalPattern() = delete;
    /// @brief Copy c-tor
    GlobalPattern(const GlobalPattern& other) = default;

    /// @brief Set the average theta of the pattern
    void setTheta(double theta) { m_theta = theta; }
    /// @brief Set the average phi of the pattern
    void setPhi(double phi) { m_phi = phi; }
    /// @brief Set the main sector of the pattern    
    void setSector(int sector) { m_sector1 = sector; }
    /// @brief Set the associated sector to the bucket in case of overlap
    void setSecondarySector(int sector) { m_sector2 = sector; }
    /// @brief Set the number of precision hits in the pattern
    void setNPrecisionHits(unsigned n) { m_nPrecisionHits = n; }
    /// @brief Set the number of eta non-precision hits in the pattern
    void setNEtaNonPrecisionHits(unsigned n) { m_nEtaNonPrecisionHits = n; }
    /// @brief Set the number of phi hits in the pattern
    void setNPhiHits(unsigned n) { m_nPhiHits = n; }
    /// @brief Total residual of the pattern from pattern finding
    void setTotalResidual(double res) { m_totalResidual = res; }
    

    /// @brief Return the average global theta of the pattern
    double theta() const { return m_theta; }
    /// @brief Return the average global phi of the pattern
    double phi() const { return m_phi; }
    /// @brief Return the main sector where the pattern is located
    int sector() const { return m_sector1; }
    /// @brief Return whether the pattern is located in the overlap region between two sectors
    bool isSectorOverlap() const { return m_sector2 != m_sector1; }
    /// @brief Return the associated sector to the bucket
    int secondarySector() const { return m_sector2; }
    /// @brief Return the sector phi of the pattern. It is the central phi of the sector or the the value at the edge in case of overlap
    double sectorPhi() const;
    /// @brief Return the associated stations to the pattern
    std::vector<StIndex> getStations() const;
    /// @brief Return the pattern hits in the given station
    const std::vector<HitType>& hitsInStation(StIndex station) const;
    /// @brief Return the number of precision hits in the pattern
    unsigned nPrecisionHits() const { return m_nPrecisionHits; }
    /// @brief Return the number of eta non-precision hits in the pattern
    unsigned nEtaNonPrecisionHits() const { return m_nEtaNonPrecisionHits; }
    /// @brief Return the number of phi hits in the pattern
    unsigned nPhiHits() const { return m_nPhiHits; }
    /// @brief Return the total residual of the pattern from pattern finding
    double totalResidual() const { return m_totalResidual; }
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
    /** Number of precision hits */
    unsigned m_nPrecisionHits{0};
    /** Number of eta non-precision measurements */
    unsigned m_nEtaNonPrecisionHits{0};
    /** Number of phi measurements */
    unsigned m_nPhiHits{0};
    /** Total residual of the pattern from pattern finding */
    double m_totalResidual{0.};

    // The pattern can extend over two sectors in the overlap region
    int m_sector1{-1};
    int m_sector2{-1};

    /** Hits of the pattern organized per station */
    const HitCollection m_hitsInStation{};
};
/** @brief Abrivation of the GlobalPattern container type */
using GlobalPatternContainer = DataVector<GlobalPattern>;
}  // namespace MuonR4

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( MuonR4::GlobalPatternContainer , 1197530715 , 1 )

#endif
