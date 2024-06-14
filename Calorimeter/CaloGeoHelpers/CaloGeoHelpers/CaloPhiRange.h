/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file CaloGeoHelpers/CaloPhiRange.h
 *
 * @brief CaloPhiRange class declaration
 *
 */

#ifndef CALOGEOHELPER_CALOPHIRANGE_H
#define CALOGEOHELPER_CALOPHIRANGE_H

#include <numbers>
/** @class CaloPhiRange
 *
 *  @brief This class defines the phi convention for Calorimeters
 *
 *       up to Release 8.3.0 (included) : 0->2pi
 *
 *       as of 8.4.0 : -pi -> pi
 *
 */


class CaloPhiRange
{
public:
    static double twopi ();
    static double phi_min ();
    static double phi_max ();

    static double fix ( double phi );

    /** @brief simple phi1 - phi2 calculation, but result is fixed to respect range.
     */
    static double diff ( double phi1,  double phi2 );

private:
    constexpr static const double m_phi_min = -std::numbers::pi;
    constexpr static const double m_twopi = 2*std::numbers::pi;
    constexpr static const double m_phi_max = std::numbers::pi;
};

inline double CaloPhiRange::twopi()
{ return m_twopi;}

inline double CaloPhiRange::phi_min()
{ return m_phi_min;}

inline double CaloPhiRange::phi_max()
{ return m_phi_max;}

#endif // CALODETDESCR_CALOPHIRANGE_H
