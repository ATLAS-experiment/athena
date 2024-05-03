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
#include <cmath> // std::fabs, std::copysign


namespace IDTPM {

  /// Accessor utility function for getting the value of pT
  template< class U >
  inline float pT( const U& p ) { return p.pt(); }

  /// Accessor utility function for getting the value of signed pT
  template< class U >
  inline float pTsig( const U& p ) {
    return p.charge() ? std::copysign( pT(p), p.charge() ) : 0.;
  }

  /// Accessor utility function for getting the value of eta
  template< class U >
  inline float eta( const U& p ) { return p.eta(); }

  /// Accessor utility function for getting the value of phi
  template< class U >
  inline float phi( const U& p ) { return p.phi(); }

  /// Accessor utility function for getting the value of z0
  inline float getZ0( const xAOD::TrackParticle& p ) { return p.z0(); }
  inline float getZ0( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("z0") ) ?
           p.auxdata<float>("z0") : -9999.;
  }
  template< class U >
  inline float z0( const U& p ) { return getZ0( p ); }

  /// Accessor utility function for getting the value of d0
  inline float getD0( const xAOD::TrackParticle& p ) { return p.d0(); }
  inline float getD0( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("d0") ) ?
           p.auxdata<float>("d0") : -9999.;
  }
  template< class U >
  inline float d0( const U& p ) { return getD0( p ); }

  /// Accessor utility function for getting the value of qOverP
  inline float getQoverP( const xAOD::TrackParticle& p ) { return p.qOverP(); }
  inline float getQoverP( const xAOD::TruthParticle& p ) {
    return ( p.isAvailable<float>("qOverP") ) ?
           p.auxdata<float>("qOverP") : -9999.;
  }
  template< class U >
  inline float qOverP( const U& p ) { return getQoverP( p ); }

  /// Accessor utility function for getting the value of Energy
  template< class U >
  inline float eTot( const U& p ) { return p.e(); }

  /// Accessor utility function for getting the value of Tranverse energy
  template< class U >
  inline float eT( const U& p ) { return p.p4().Et(); }

  /// Accessor utility function for getting the DeltaPhi betwen two tracks
  template< class U1, class U2=U1 >
  inline float deltaPhi( const U1& p1, const U2& p2 ) {
    return p1.p4().DeltaPhi( p2.p4() );
  }

  /// Accessor utility function for getting the DeltaEta betwen two tracks
  template< class U1, class U2=U1 >
  inline float deltaEta( const U1& p1, const U2& p2 ) {
    return ( eta(p1) - eta(p2) );
  }

  /// Accessor utility function for getting the DeltaR betwen two tracks
  template< class U1, class U2=U1 >
  inline float deltaR( const U1& p1, const U2& p2 ) {
    return p1.p4().DeltaR( p2.p4() );
  }

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_TRKPARAMETERSHELPER_H
