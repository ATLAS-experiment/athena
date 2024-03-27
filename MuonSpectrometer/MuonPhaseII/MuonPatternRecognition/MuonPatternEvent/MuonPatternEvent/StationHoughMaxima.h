/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_STATIONHOUGHMAXIMA__H
#define MUONR4_STATIONHOUGHMAXIMA__H

#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"
namespace MuonR4{

  /// @brief Small data class to collect the hough maxima for one given station.
  /// Contains a list of maxima and a station identifier. 
  /// Sorts by station identifier to allow set / map insertion 
  class StationHoughMaxima{
    public:
         /// constructor
         /// @param chamber: Associated chamber serving as Identifier
         /// @param maxima: list of maxima (can be extended later) 
        StationHoughMaxima(const MuonGMR4::MuonChamber* chamber, 
                           const std::vector<HoughMaximum> & maxima = {});
        
        /// adds a maximum to the list
        /// @param m: Maximum to add
        void addMaximum(const HoughMaximum & m); 

        /// @brief Returns the associated chamber
        const MuonGMR4::MuonChamber* chamber() const;

        /// @brief getter
        /// @return the maxima for this station
        const std::vector<HoughMaximum> & getMaxima() const;

        /// @brief sorting operator - uses identifiers for sorting, not the maxima themselves
        /// @param other: station maxima list to compare to
        /// @return Comparison between the station identifiers
        bool operator< (const StationHoughMaxima & other) const;

    private:
        const MuonGMR4::MuonChamber* m_chamber{};     // the identifier for this station
        std::vector<HoughMaximum> m_maxima{};         // the list of found maxima
};

}

#endif
