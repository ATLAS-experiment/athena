/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRKPARAMETERSHELPER_H
#define INDETTRACKPERFMON_TRKPARAMETERSHELPER_H

/**
 * @file TrackParametersHelper.h
 * @brief Utility methods to access 
 *        track/truth particles parmeters in
 *        a consitent way in this package
 * @author Marco Aparo <marco.aparo@cern.ch>
 * @date 25 September 2023
 **/

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

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

  /// Accessor utility function for getting the value of theta
  inline float getTheta( const xAOD::TrackParticle& p ) { return p.theta(); }
  inline float getTheta( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> thetaAcc("theta");
    return (thetaAcc.isAvailable(p)) ? thetaAcc(p) : -9999.;
  }
  template< class U >
  inline float theta( const U& p ) { return getTheta( p ); }

  /// Accessor utility function for getting the value of phi
  template< class U >
  inline float phi( const U& p ) { return p.phi(); }

  /// Accessor utility function for getting the value of z0
  inline float getZ0( const xAOD::TrackParticle& p ) { return p.z0(); }
  inline float getZ0( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> z0Acc("z0");
    return (z0Acc.isAvailable(p)) ? z0Acc(p) : -9999.;
  }
  template< class U >
  inline float z0( const U& p ) { return getZ0( p ); }

  template< class U >
  inline float z0SinTheta( const U& p ) { return z0( p ) * std::sin( theta( p ) ); }

  /// Accessor utility function for getting the value of d0
  inline float getD0( const xAOD::TrackParticle& p ) { return p.d0(); }
  inline float getD0( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> d0Acc("d0");
    return (d0Acc.isAvailable(p)) ? d0Acc(p) : -9999.;
  }
  template< class U >
  inline float d0( const U& p ) { return getD0( p ); }

  /// Accessor utility function for getting the value of R
  inline float getProdR( const xAOD::TrackParticle& ) { return -9999.; } //FIXME
  inline float getProdR( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> prodRAcc("prodR");
    return (prodRAcc.isAvailable(p)) ? prodRAcc(p) : -9999.;
  }
  template< class U >
  inline float prodR( const U& p ) { return getProdR( p ); }

  /// Accessor utility function for getting the value of Z
  inline float getProdZ( const xAOD::TrackParticle& ) { return -9999.; } //FIXME
  inline float getProdZ( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> prodZAcc("prodZ");
    return (prodZAcc.isAvailable(p)) ? prodZAcc(p) : -9999.;
  }
  template< class U >
  inline float prodZ( const U& p ) { return getProdZ( p ); }

  /// Accessor utility function for getting the value of prodR
  inline float getR( const xAOD::TrackParticle& ) { return -9999.; } //FIXME
  inline float getR( const xAOD::TruthParticle& p ) {
    return ( p.hasProdVtx() ) ?
           (*p.prodVtx()).perp() : -9999.;
  }
  template< class U >
  inline float R( const U& p ) { return getR( p ); }

  /// Accessor utility function for getting the value of prodZ
  inline float getZ( const xAOD::TrackParticle& ) { return -9999.; } //FIXME
  inline float getZ( const xAOD::TruthParticle& p ) {
    return ( p.hasProdVtx() ) ?
           (*p.prodVtx()).z() : -9999.;
  }
  template< class U >
  inline float Z( const U& p ) { return getZ( p ); }

  /// Accessor utility function for getting the value of qOverP
  inline float getQoverP( const xAOD::TrackParticle& p ) { return p.qOverP(); }
  inline float getQoverP( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> qOverPAcc("qOverP");
    return (qOverPAcc.isAvailable(p)) ? qOverPAcc(p) : -9999.;
  }
  template< class U >
  inline float qOverP( const U& p ) { return getQoverP( p ); }

  template< class U >
  inline float qOverPT( const U& p ) { return qOverP( p ) / std::sin( theta( p ) ); }

  /// Accessor utility function for getting the value of Energy
  template< class U >
  inline float eTot( const U& p ) { return p.e(); }

  /// Accessor utility function for getting the value of Tranverse energy
  template< class U >
  inline float eT( const U& p ) { return p.p4().Et(); }

  /// Accessor utility function for getting the value of chi^2
  inline float getChiSquared( const xAOD::TrackParticle& p ) { return p.chiSquared(); }
  inline float getChiSquared( const xAOD::TruthParticle& ) { return -9999; }
  template< class U >
  inline float chiSquared( const U& p ) { return getChiSquared(p); }

  /// Accessor utility function for getting the value of #dof
  inline float getNdof( const xAOD::TrackParticle& p ) { return p.numberDoF(); }
  inline float getNdof( const xAOD::TruthParticle& ) { return -9999; }
  template< class U >
  inline float ndof( const U& p ) { return getNdof(p); }

  /// Accessor utilify function for track parameter uncertainty
  inline float getTrackParameterError( const xAOD::TrackParticle& p, Trk::ParamDefs param ) {
    return std::sqrt(p.definingParametersCovMatrix()(param, param));
  }
  inline float getTrackParameterError( const xAOD::TruthParticle&, Trk::ParamDefs ) { return -9999; }
  template< class U >
  inline float trackParameterError( const U& p, Trk::ParamDefs param ) { return getTrackParameterError(p, param); }

  inline float getQOverPtError( const xAOD::TrackParticle& p ) {
    float inverseSinTheta = 1./std::sin(p.theta());
    float cosTheta = std::cos(p.theta());
    float qOverPT_err2 =
      std::pow(trackParameterError(p, Trk::qOverP) * inverseSinTheta, 2)
      + std::pow(p.qOverP() * cosTheta * trackParameterError(p, Trk::theta) * std::pow(inverseSinTheta, 2), 2)
      - 2 * p.qOverP() * cosTheta * p.definingParametersCovMatrix()(Trk::qOverP, Trk::theta) * std::pow(inverseSinTheta, 3);
    return std::sqrt(qOverPT_err2);
  }
  inline float getQOverPtError( const xAOD::TruthParticle& ) { return -9999; }
  template< class U >
  inline float qOverPtError( const U& p ) { return getQOverPtError(p); }

  template< class U >
  inline float pTError( const U& p ) { return qOverPtError(p) * std::pow(pT(p), 2); }

  inline float getZ0SinThetaError( const xAOD::TrackParticle& p ) {
    float sinTheta = std::sin(p.theta());
    float cosTheta = std::cos(p.theta());
    float z0SinTheta_err2 =
      std::pow(trackParameterError(p, Trk::z0) * sinTheta, 2)
      + std::pow(p.z0() * cosTheta * trackParameterError(p, Trk::theta), 2)
      + 2 * p.z0() * sinTheta * cosTheta * p.definingParametersCovMatrix()(Trk::z0, Trk::theta);
    return std::sqrt(z0SinTheta_err2);
  }
  inline float getZ0SinThetaError( const xAOD::TruthParticle& ) { return -9999; }
  template< class U >
  inline float z0SinThetaError( const U& p ) { return getZ0SinThetaError(p); }

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
