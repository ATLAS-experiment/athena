/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_STATIONHOUGHMAXIMA__H
#define MUONR4_STATIONHOUGHMAXIMA__H

#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonPatternEvent/HoughSegmentSeed.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"

namespace MuonR4 {

/// @brief Small data class to collect the hough maxima for one given station.
/// Contains a list of maxima and a station identifier.
/// Sorts by station identifier to allow set / map insertion
template <class ResultType>
class StationHoughResults {
   public:
    /// constructor
    /// @param chamber: Associated chamber serving as Identifier
    /// @param maxima: list of maxima (can be extended later)
    StationHoughResults(const MuonGMR4::MuonChamber* chamber,
                        const std::vector<ResultType>& maxima = {})
        : m_chamber{chamber}, m_maxima(maxima) {}

    /// adds a maximum to the list
    /// @param m: Maximum to add
    void addMaximum(const ResultType& m) { m_maxima.push_back(m); }

    /// @brief Returns the associated chamber
    const MuonGMR4::MuonChamber* chamber() const { return m_chamber; }

    /// @brief getter
    /// @return the maxima for this station
    const std::vector<ResultType>& getMaxima() const { return m_maxima; }

    /// @brief sorting operator - uses identifiers for sorting, not the maxima
    /// themselves
    /// @param other: station maxima list to compare to
    /// @return Comparison between the station identifiers
    bool operator<(const StationHoughResults<ResultType>& other) const {
        using ChamberSorter = MuonGMR4::MuonDetectorManager::ChamberSorter;
        return ChamberSorter{}(m_chamber, other.m_chamber);
    }

   private:
    const MuonGMR4::MuonChamber*
        m_chamber{};                     // the identifier for this station
    std::vector<ResultType> m_maxima{};  // the list of found maxima
};

// specialisation for eta-maxima
using StationHoughMaxima = StationHoughResults<HoughMaximum>;
// specialisation for full segment seeds
using StationHoughSegmentSeeds = StationHoughResults<HoughSegmentSeed>;

}  // namespace MuonR4

#endif
