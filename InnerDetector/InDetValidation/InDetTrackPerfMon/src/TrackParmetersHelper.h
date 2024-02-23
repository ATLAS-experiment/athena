/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRKPARAMETERSHELPER_H
#define INDETTRACKPERFMON_TRKPARAMETERSHELPER_H

/**
 * @file TrackParmetersHelper.h
 * @brief Utility methods to access 
 *        track/truth particles parmeters in
 *        a consitent way in this package
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"

/// STD includes
#include <cmath> // std::fabs


namespace IDTPM {

  /// Accessor utility function for getting the value of pT
  template< class U >
  inline float pT( const U& p ) { return p.pt(); }

  /// Accessor utility function for getting the value of signed pT
  template< class U >
  inline float pTsig( const U& p ) {
    float pT = std::fabs( p.pt() );
    if( p.charge() < 0 )  pT *= -1;
    if( p.charge() == 0 ) pT = 0.;
    return pT;
  }

  /// Accessor utility function for getting the value of eta
  template< class U >
  inline float eta( const U& p ) { return p.eta(); }

  /// Accessor utility function for getting the value of phi
  template< class U >
  inline float phi( const U& p ) { return p.phi(); }

  /// Accessor utility function for getting the value of z0
  inline float getZ0( const xAOD::TrackParticle& p ) { return p.z0(); }
  ///
  inline float getZ0( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("z0") ) ?
           p.auxdata<float>("z0") : -9999.;
  }
  ///
  template< class U >
  inline float z0( const U& p ) { return getZ0( p ); }

  /// Accessor utility function for getting the value of d0
  inline float getD0( const xAOD::TrackParticle& p ) { return p.d0(); }
  ///
  inline float getD0( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("d0") ) ?
           p.auxdata<float>("d0") : -9999.;
  }
  ///
  template< class U >
  inline float d0( const U& p ) { return getD0( p ); }

  /// Accessor utility function for getting the value of qOverP
  inline float getQoverP( const xAOD::TrackParticle& p ) { return p.qOverP(); }
  ///
  inline float getQoverP( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("qOverP") ) ?
           p.auxdata<float>("qOverP") : -9999.;
  }
  ///
  template< class U >
  inline float qOverP( const U& p ) { return getQoverP( p ); }

  /// Accessor utility function for getting the value of Energy
  template< class U >
  inline float Etot( const U& p ) { return p.e(); }

  /// Accessor utility function for getting the value of Tranverse energy
  template< class U >
  inline float ET( const U& p ) { return p.p4().Et(); }

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_TRKPARAMETERSHELPER_H
