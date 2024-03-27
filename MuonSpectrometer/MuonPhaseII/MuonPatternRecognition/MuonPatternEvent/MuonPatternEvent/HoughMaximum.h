/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__HOUGHMAXIMUM__H
#define MUONR4__HOUGHMAXIMUM__H
#include <vector>
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MuonSpacePoint/MuonSpacePointContainer.h"

namespace MuonR4{
    using HoughHitType = std::shared_ptr<MuonR4::MuonSpacePoint>;
    /// @brief Data class to represent one maximum in hough space. 
    /// For generic usage, local coordinates are referred to as "x" and "y"
    /// but may correspond to any frame 
    class HoughMaximum{
        public:
            /// @brief constructor. 
            /// @param x: first coordinate of maximum
            /// @param y: second coordinate of maximum
            /// @param counts: Size of the maximum (typically, weighted hough hit counts) - can use to sort
            /// @param hits: list of hits assigned to the maximum (owned by SG). 
            HoughMaximum(double x,  
                         double y, 
                         double counts, 
                         std::vector<HoughHitType> && hits);
            /// @brief default c-tor, creates empty maximum with zero counts in the origin
            HoughMaximum() = default; 
            /// @brief getter
            /// @return  the first coordinate
            double getX() const; 
            /// @brief getter
            /// @return  the second coordinate
            double getY() const; 
            /// @brief getter
            /// @return  the uncertainty on the first coordinate
            double getCounts() const; 
            /// @brief getter 
            /// @return  the hits assigned to this maximum
            const std::vector<HoughHitType> & getHitsInMax() const; 

        private:
            double m_x{0.};     // first coordinate
            double m_y{0.};     // second coordinate
            double m_counts{0.};    // weighted counts 
            std::vector<HoughHitType> m_hitsInMax{};   // list of hits on maximum
    };
}

#endif
