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

/// STD includes
#include <vector>
#include <cmath> // std::fabs, std::copysign

/// local includes
#include "VertexParametersHelper.h"


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
  inline float getPhi( const xAOD::TrackParticle& p ) { return p.phi0(); }
  inline float getPhi( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> phiAcc("phi");
    return (phiAcc.isAvailable(p)) ? phiAcc(p) : -9999.;
  }
  template< class U >
  inline float phi( const U& p ) { return getPhi( p ); }
  //template< class U >
  //inline float phi( const U& p ) { return p.phi(); }

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
  inline float getProdR( const xAOD::TrackParticle& ) { return -9999.; }
  inline float getProdR( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> prodRAcc("prodR");
    return (prodRAcc.isAvailable(p)) ? prodRAcc(p) : -9999.;
  }
  template< class U >
  inline float prodR( const U& p ) { return getProdR( p ); }

  /// Accessor utility function for getting the value of Z
  inline float getProdZ( const xAOD::TrackParticle& ) { return -9999.; }
  inline float getProdZ( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> prodZAcc("prodZ");
    return (prodZAcc.isAvailable(p)) ? prodZAcc(p) : -9999.;
  }
  template< class U >
  inline float prodZ( const U& p ) { return getProdZ( p ); }

  /// Accessor utility function for getting the value of qOverP
  inline float getQoverP( const xAOD::TrackParticle& p ) { return p.qOverP(); }
  inline float getQoverP( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> qOverPAcc("qOverP");
    return (qOverPAcc.isAvailable(p)) ? qOverPAcc(p) : -9999.;
  }
  template< class U >
  inline float qOverP( const U& p ) { return getQoverP( p ); }

  template< class U >
  inline float qOverPT( const U& p ) { return theta(p)==0 ? -9999. : qOverP(p) / std::sin( theta(p) ); }

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

  /// Accessor utility function for getting the track author
  inline std::vector< unsigned int > getAuthor( const xAOD::TrackParticle& p ) {
    std::vector< unsigned int > authorVec;
    std::bitset< xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo > patternInfo = p.patternRecoInfo();
    for( unsigned int i = 0 ; i < xAOD::TrackPatternRecoInfo::NumberOfTrackRecoInfo ; i++ ) {
      if( patternInfo.test(i) ) authorVec.push_back(i);
    }
    return authorVec;
  }
  inline std::vector< unsigned int > getAuthor( const xAOD::TruthParticle& ) { return {}; }
  template< class U >
  inline std::vector< unsigned int > author( const U& p ) { return getAuthor(p); }

  /// Accessor utility function for getting the track hasValidTime
  inline uint8_t getHasValidTime( const xAOD::TrackParticle& p ) { return p.hasValidTime(); }
  inline uint8_t getHasValidTime( const xAOD::TruthParticle& ) { return 0; }
  template< class U >
  inline uint8_t hasValidTime( const U& p ) { return getHasValidTime(p); }

  /// Accessor utility function for getting the track time
  inline float getTime( const xAOD::TrackParticle& p ) { return p.time(); }
  inline float getTime( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float time( const U& p ) { return getTime(p); }

  /// Accessor utility function for getting the track parameters covariance
  inline float getCov( const xAOD::TrackParticle& p, Trk::ParamDefs par1, Trk::ParamDefs par2 ) {
    return p.definingParametersCovMatrix()( par1, par2 ); }
  inline float getCov( const xAOD::TruthParticle&, Trk::ParamDefs, Trk::ParamDefs ) { return 0.; }
  template< class U >
  inline float cov( const U& p, Trk::ParamDefs par1, Trk::ParamDefs par2 ) {
    return getCov( p, par1, par2 ); }

  /// Accessor utility function for getting the track parameters error
  inline float getError( const xAOD::TrackParticle& p, Trk::ParamDefs par ) {
    return ( cov(p, par, par) < 0 ) ? 0. : std::sqrt( cov(p, par, par) ); }
  inline float getError( const xAOD::TruthParticle&, Trk::ParamDefs ) { return 0.; }
  template< class U >
  inline float error( const U& p, Trk::ParamDefs par ) { return getError( p, par ); }

  /// Accessor utility function for getting the track parameters covariance vector
  inline std::vector< float > getCovVec( const xAOD::TrackParticle& p ) {
    return p.definingParametersCovMatrixVec(); }
  inline std::vector< float > getCovVec( const xAOD::TruthParticle& ) { return {}; }
  template< class U >
  inline std::vector< float > covVec( const U& p ) { return getCovVec( p ); }

  /// Accessor utility function for getting the QOverPt error
  inline float getQOverPTError( const xAOD::TrackParticle& p ) {
    float invSinTheta = 1. / std::sin( theta(p) );
    float cosTheta = std::cos( theta(p) );
    float qOverPTerr2 =
      std::pow( error(p, Trk::qOverP) * invSinTheta, 2 )
      + std::pow( error(p, Trk::theta) * qOverP(p) * cosTheta * std::pow(invSinTheta, 2), 2 )
      - 2 * qOverP(p) * cosTheta * cov(p, Trk::theta, Trk::qOverP) * std::pow(invSinTheta, 3);
    return qOverPTerr2 > 0 ? std::sqrt( qOverPTerr2 ) : 0.;
  }
  inline float getQOverPTError( const xAOD::TruthParticle& ) { return 0.; }
  template< class U >
  inline float qOverPTError( const U& p ) { return getQOverPTError(p); }

  /// Accessor utility function for getting the Pt error
  inline float getPTError( const xAOD::TrackParticle& p ) {
    std::vector< float > covs = covVec(p);
    if( covs.size() < 15 ) {
      throw std::runtime_error(
        "TrackParticle without covariance matrix for defining parameters or the covariance matrix is wrong dimensionality.");
      return 0.;
    }
    if( qOverP(p) <= 0. ) return 0.;
    float diff_qp = - pT(p) / std::fabs( qOverP(p) );
    float diff_theta = theta(p) == 0. ? 0. : pT(p) / std::tan( theta(p) );
    float pTerr2 = diff_qp * (diff_qp * covs[14] + diff_theta * covs[13]) + diff_theta * diff_theta * covs[9];
    return pTerr2 > 0. ? std::sqrt( pTerr2 ) : 0.;
  }
  inline float getPTError( const xAOD::TruthParticle& ) { return 0.; }
  template< class U >
  inline float pTError( const U& p ) { return getPTError(p); }

  /// Accessor utility function for getting the Eta error
  inline float getEtaError( const xAOD::TrackParticle& p ) {
    float etaErr =
      error(p, Trk::theta) / ( -2 * std::sin( theta(p) ) * std::cos( theta(p) ) );
    return std::fabs( etaErr ); }
  inline float getEtaError( const xAOD::TruthParticle& ) { return 0.; }
  template< class U >
  inline float etaError( const U& p ) { return getEtaError(p); }

  /// Accessor utility function for getting the z0SinTheta error
  inline float getZ0SinThetaError( const xAOD::TrackParticle& p ) {
    float z0sinErr2 =
      std::pow( error(p, Trk::z0) * std::sin( theta(p) ), 2 )
      + std::pow( z0(p) * error(p, Trk::theta) * std::cos( theta(p) ), 2)
      + 2 * z0(p) * std::sin( theta(p) ) * std::cos( theta(p) ) * cov(p, Trk::z0, Trk::theta);
    return z0sinErr2 > 0. ? std::sqrt( z0sinErr2 ) : 0.;
  }
  inline float getZ0SinThetaError( const xAOD::TruthParticle& ) { return 0.; }
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
  
  /// Accessor utility function for getting the value of isHadron
  inline float getIsHadron( const xAOD::TrackParticle& ) { return 0; }
  inline float getIsHadron( const xAOD::TruthParticle& p ) { return p.isHadron();}
  template< class U >
  inline float isHadron( const U& p ) { return getIsHadron( p ); }

  /// Accessor utility function for getting the value of isPion
  inline float getIsPion( const xAOD::TrackParticle& ) { return 0; }
  inline float getIsPion( const xAOD::TruthParticle& p ) { return (p.pdgId() == 111 or p.pdgId() == 211);}
  template< class U >
  inline float isPion( const U& p ) { return getIsPion( p ); }


  /// Accessor utility function for getting the value of truthType
  inline int getTruthType( const xAOD::TrackParticle& ) { return -9999; }
  inline int getTruthType( const xAOD::TruthParticle& p ) {
    SG::ConstAccessor<int> type( "Truth_truthType" );
    return type.isAvailable(p) ? type(p) : -9999;
  }
  template< class U >
  inline int truthType( const U& p ) { return getTruthType( p ); }

  /// Accessor utility function for getting the value of truthOrigin
  inline int getTruthOrigin( const xAOD::TrackParticle& ) { return -9999; }
  inline int getTruthOrigin( const xAOD::TruthParticle& p ) {
    SG::ConstAccessor<int> origin( "Truth_truthOrigin" );
    return origin.isAvailable(p) ? origin(p) : -9999;
  }
  template< class U >
  inline int truthOrigin( const U& p ) { return getTruthOrigin( p ); }


  /// Accessor utility function for getting the value of nInnerMostPixelHits
  inline float getNInnerMostPixelHits( const xAOD::TrackParticle& p ) {
    uint8_t iInnerMostPixelHits(0);
    return p.summaryValue( iInnerMostPixelHits, xAOD::numberOfInnermostPixelLayerHits ) ?
           float( iInnerMostPixelHits ) : -9999.;
  }
  inline float getNInnerMostPixelHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nInnerMostPixelHits( const U& p ) { return getNInnerMostPixelHits( p ); }

  /// Accessor utility function for getting the value of nInnerMostPixelEndcapHits
  inline float getNInnerMostPixelEndcapHits( const xAOD::TrackParticle& p ) {
    uint8_t iInnerMostPixelEndcapHits(0);
    return p.summaryValue( iInnerMostPixelEndcapHits, xAOD::numberOfInnermostPixelLayerEndcapHits ) ?
           float( iInnerMostPixelEndcapHits ) : -9999.;
  }
  inline float getNInnerMostPixelEndcapHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nInnerMostPixelEndcapHits( const U& p ) { return getNInnerMostPixelEndcapHits( p ); }

  /// Accessor utility function for getting the value of nNextToInnerMostPixelHits
  inline float getNNextToInnerMostPixelHits( const xAOD::TrackParticle& p ) {
    uint8_t iNextToInnerMostPixelHits(0);
    return p.summaryValue( iNextToInnerMostPixelHits, xAOD::numberOfNextToInnermostPixelLayerHits ) ?
           float( iNextToInnerMostPixelHits ) : -9999.;
  }
  inline float getNNextToInnerMostPixelHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nNextToInnerMostPixelHits( const U& p ) { return getNNextToInnerMostPixelHits( p ); }

  /// Accessor utility function for getting the value of nNextToInnerMostPixelEndcapHits
  inline float getNNextToInnerMostPixelEndcapHits( const xAOD::TrackParticle& p ) {
    uint8_t iNextToInnerMostPixelEndcapHits(0);
    return p.summaryValue( iNextToInnerMostPixelEndcapHits, xAOD::numberOfNextToInnermostPixelLayerEndcapHits ) ?
           float( iNextToInnerMostPixelEndcapHits ) : -9999.;
  }
  inline float getNNextToInnerMostPixelEndcapHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nNextToInnerMostPixelEndcapHits( const U& p ) { return getNNextToInnerMostPixelEndcapHits( p ); }

  /// Accessor utility function for getting the value of nInnerMostPixelSharedHits
  inline float getNInnerMostPixelSharedHits( const xAOD::TrackParticle& p ) {
    uint8_t iInnerMostPixelSharedHits(0);
    return p.summaryValue( iInnerMostPixelSharedHits, xAOD::numberOfInnermostPixelLayerSharedHits ) ?
           float( iInnerMostPixelSharedHits ) : -9999.;
  }
  inline float getNInnerMostPixelSharedHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nInnerMostPixelSharedHits( const U& p ) { return getNInnerMostPixelSharedHits( p ); }

  /// Accessor utility function for getting the value of nInnerMostPixelSharedEndcapHits
  inline float getNInnerMostPixelSharedEndcapHits( const xAOD::TrackParticle& p ) {
    uint8_t iInnerMostPixelSharedEndcapHits(0);
    return p.summaryValue( iInnerMostPixelSharedEndcapHits, xAOD::numberOfInnermostPixelLayerSharedEndcapHits ) ?
           float( iInnerMostPixelSharedEndcapHits ) : -9999.;
  }
  inline float getNInnerMostPixelSharedEndcapHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nInnerMostPixelSharedEndcapHits( const U& p ) { return getNInnerMostPixelSharedEndcapHits( p ); }

  /// Accessor utility function for getting the value of nPixelHits
  inline float getNPixelHits( const xAOD::TrackParticle& p ) {
    uint8_t iPixelHits(0);
    return p.summaryValue( iPixelHits, xAOD::numberOfPixelHits ) ?
           float( iPixelHits ) : -9999.;
  }
  inline float getNPixelHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nPixelHits( const U& p ) { return getNPixelHits( p ); }

  /// Accessor utility function for getting the value of nPixelHoles
  inline float getNPixelHoles( const xAOD::TrackParticle& p ) {
    uint8_t iPixHoles(0);
    return p.summaryValue( iPixHoles, xAOD::numberOfPixelHoles ) ?
           float( iPixHoles ) : -9999.;
  }
  inline float getNPixelHoles( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nPixelHoles( const U& p ) { return getNPixelHoles( p ); }

  /// Accessor utility function for getting the value of nPixelSharedHits
  inline float getNPixelSharedHits( const xAOD::TrackParticle& p ) {
    uint8_t iPixelShared(0);
    return p.summaryValue( iPixelShared, xAOD::numberOfPixelSharedHits ) ?
           float( iPixelShared ) : -9999.;
  }
  inline float getNPixelSharedHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nPixelSharedHits( const U& p ) { return getNPixelSharedHits( p ); }

  /// Accessor utility function for getting the value of pixeldEdx
  inline float getPixeldEdx( const xAOD::TrackParticle& p ) {
    float iPixeldEdx(0);
    return p.summaryValue( iPixeldEdx, xAOD::pixeldEdx ) ? iPixeldEdx : -9999.;
  }
  inline float getPixeldEdx( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float pixeldEdx( const U& p ) { return getPixeldEdx( p ); }

  /// Accessor utility function for getting the value of nSCTHits
  inline float getNSCTHits( const xAOD::TrackParticle& p ) {
    uint8_t iSctHits(0);
    return p.summaryValue( iSctHits, xAOD::numberOfSCTHits ) ?
           float( iSctHits ) : -9999.;
  }
  inline float getNSCTHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nSCTHits( const U& p ) { return getNSCTHits( p ); }

  /// Accessor utility function for getting the value of nSCTHoles
  inline float getNSCTHoles( const xAOD::TrackParticle& p ) {
    uint8_t iSctHoles(0);
    return p.summaryValue( iSctHoles, xAOD::numberOfSCTHoles ) ?
           float( iSctHoles ) : -9999.;
  }
  inline float getNSCTHoles( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nSCTHoles( const U& p ) { return getNSCTHoles( p ); }

  /// Accessor utility function for getting the value of nSCTSharedHits
  inline float getNSCTSharedHits( const xAOD::TrackParticle& p ) {
    uint8_t iSCTShared(0);
    return p.summaryValue( iSCTShared, xAOD::numberOfSCTSharedHits ) ?
           float( iSCTShared ) : -9999.;
  }
  inline float getNSCTSharedHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nSCTSharedHits( const U& p ) { return getNSCTSharedHits( p ); }

  /// Accessor utility function for getting the value of nSiHits
  inline float getNSiHits( const xAOD::TrackParticle& p ) {
    if( nPixelHits(p)<0 or nSCTHits(p)<0 ) return -9999.;
    float pnPixelHits = ( nPixelHits(p) >= 0 ) ? nPixelHits(p) : 0.;
    float pnSCTHits =   ( nSCTHits(p) >= 0 ) ? nSCTHits(p) : 0.;
    return ( pnPixelHits + pnSCTHits );
  }
  inline float getNSiHits( const xAOD::TruthParticle& p ) {
    static thread_local SG::ConstAccessor<float> prodNSiHits( "nSilHits" );
    return prodNSiHits.isAvailable(p) ? prodNSiHits(p) : -9999.;
  }
  template< class U >
  inline float nSiHits( const U& p ) { return getNSiHits( p ); }

  /// Accessor utility function for getting the value of nSiHoles
  template< class U >
  inline float nSiHoles( const U& p ) {
    if( nPixelHoles(p)<0 or nSCTHoles(p)<0 ) return -9999.;
    float pnPixelHoles = ( nPixelHoles(p) >= 0 ) ? nPixelHoles(p) : 0.;
    float pnSCTHoles =   ( nSCTHoles(p) >= 0 ) ? nSCTHoles(p) : 0.;
    return ( pnPixelHoles + pnSCTHoles );
  }

  /// Accessor utility function for getting the value of nTRTHits
  inline float getNTRTHits( const xAOD::TrackParticle& p ) {
    uint8_t iTrtHits(0);
    return p.summaryValue( iTrtHits, xAOD::numberOfTRTHits ) ?
           float( iTrtHits ) : -9999.;
  }
  inline float getNTRTHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHits( const U& p ) { return getNTRTHits( p ); }

  /// Accessor utility function for getting the value of nTRTHitsXe
  inline float getNTRTHitsXe( const xAOD::TrackParticle& p ) {
    uint8_t iTrtHitsXe(0);
    return p.summaryValue( iTrtHitsXe, xAOD::numberOfTRTXenonHits ) ?
           float( iTrtHitsXe ) : -9999.;
  }
  inline float getNTRTHitsXe( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHitsXe( const U& p ) { return getNTRTHitsXe( p ); }

  /// Accessor utility function for getting the value of nTRTHitsAr
  inline float getNTRTHitsAr( const xAOD::TrackParticle& p ) {
    if( nTRTHits(p)<0 or nTRTHitsXe(p)<0 ) return -9999.;
    float pnTRTHits   = ( nTRTHits(p) >= 0 ) ? nTRTHits(p) : 0.;
    float pnTRTHitsXe = ( nTRTHitsXe(p) >= 0 ) ? nTRTHitsXe(p) : 0.;
    return ( pnTRTHits - pnTRTHitsXe );
  }
  inline float getNTRTHitsAr( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHitsAr( const U& p ) { return getNTRTHitsAr( p ); }

  /// Accessor utility function for getting the value of nTRTHighThresholdHits
  inline float getNTRTHighThresholdHits( const xAOD::TrackParticle& p ) {
    uint8_t iTrtHighThresholdHits(0);
    return p.summaryValue( iTrtHighThresholdHits, xAOD::numberOfTRTHighThresholdHitsTotal ) ?
           float( iTrtHighThresholdHits ) : -9999.;
  }
  inline float getNTRTHighThresholdHits( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHighThresholdHits( const U& p ) { return getNTRTHighThresholdHits( p ); }

  /// Accessor utility function for getting the value of nTRTHighThresholdHitsXe
  inline float getNTRTHighThresholdHitsXe( const xAOD::TrackParticle& p ) {
    uint8_t iTrtHighThresholdHitsXe(0);
    return p.summaryValue( iTrtHighThresholdHitsXe, xAOD::numberOfTRTHighThresholdHits ) ?
           float( iTrtHighThresholdHitsXe ) : -9999.;
  }
  inline float getNTRTHighThresholdHitsXe( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHighThresholdHitsXe( const U& p ) { return getNTRTHighThresholdHitsXe( p ); }

  /// Accessor utility function for getting the value of nTRTHighThresholdHitsAr
  inline float getNTRTHighThresholdHitsAr( const xAOD::TrackParticle& p ) {
    if( nTRTHighThresholdHits(p)<0 or nTRTHighThresholdHitsXe(p)<0 ) return -9999.;
    float pnTRTHighThresholdHits   = ( nTRTHighThresholdHits(p) >= 0 ) ? nTRTHighThresholdHits(p) : 0.;
    float pnTRTHighThresholdHitsXe = ( nTRTHighThresholdHitsXe(p) >= 0 ) ? nTRTHighThresholdHitsXe(p) : 0.;
    return ( pnTRTHighThresholdHits - pnTRTHighThresholdHitsXe );
  }
  inline float getNTRTHighThresholdHitsAr( const xAOD::TruthParticle& ) { return -9999.; }
  template< class U >
  inline float nTRTHighThresholdHitsAr( const U& p ) { return getNTRTHighThresholdHitsAr( p ); }

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_TRKPARAMETERSHELPER_H
