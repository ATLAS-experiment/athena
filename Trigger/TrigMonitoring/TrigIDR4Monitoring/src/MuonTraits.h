// emacs: this is -*- c++ -*-
/**
 **   @file    MuonTraits.h        
 **                   
 **   @author  sutt
 **   @date    Thursday 12 Mar 2026 12:10:21 GMT
 **
 **   $Id: MuonTraits.h, v0.0   Thu 12 Mar 2026 12:10:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef  IDTPM2_MUONTRAITS_H
#define  IDTPM2_MUONTRAITS_H

#include "xAODMuon/Muon.h"

#include "TrackTraits.h"


template<>
struct TrackTraits<xAOD::Muon> : public TrackTraitsBase<xAOD::Muon> {
  
public:

  static constexpr xAOD::Muon::TrackParticleType muontype() { return xAOD::Muon::InnerDetectorTrackParticle; }
  /// do we instead really want the combined muon ???
  // static constexpr xAOD::Muon::TrackParticleType muontype() { return xAOD::Muon::CombinedTrackParticle; }
  
  static uint8_t summary_value( const xAOD::Muon& m, xAOD::SummaryType type ) {
    uint8_t n(0); m.trackParticle(muontype())->summaryValue(n, type); return n;
  }
  
  //  static float     pt(const xAOD::Muon& m) { return m.trackParticle(muontype())->pt(); }
  static float     pt(const xAOD::Muon& m) { return m.pt(); }
  static float    phi(const xAOD::Muon& m) { return m.phi();    } 
  static float    eta(const xAOD::Muon& m) { return m.eta();    } 
  static float  theta(const xAOD::Muon& m) { return 2*std::atan(std::exp(-m.eta()));  }
  
  static float     z0(const xAOD::Muon& m) { return m.trackParticle(muontype())->z0() + m.trackParticle(muontype())->vz(); }

  static float     d0(const xAOD::Muon& m) { return m.trackParticle(muontype())->d0(); }

  static float      m(const xAOD::Muon& m) { return m.m(); }

  static float      E(const xAOD::Muon& m) { return m.e(); }

  static float     ET(const xAOD::Muon& m) { return m.e()*std::sin(theta(m)); }

  // Check this I do not trust the way I changed it 
  static float      p(const xAOD::Muon& m) { 
    if ( m.trackParticle(muontype())->qOverP()==0 ) throw std::runtime_error( "divide by 0 - track qOverP() is 0" );
    return 1/m.trackParticle(muontype())->qOverP();
  }

  static float qOverP(const xAOD::Muon& m) { return m.trackParticle(muontype())->qOverP(); }

  static float     dphi(const xAOD::Muon& m) { return std::sqrt(m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::phi0,Trk::phi0)); }
  static float   dtheta(const xAOD::Muon& m) { return std::sqrt(m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::theta,Trk::theta)); }
  static float  dqOverP(const xAOD::Muon& m) { return std::sqrt(m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP)); }
  
  static float     deta(const xAOD::Muon& m) { return 0.5*dtheta(m)/(std::cos(0.5*theta(m))*std::cos(0.5*theta(m))*std::tan(0.5*theta(m))); }  // ???? CHECK THIS <<--

  static float      dz0(const xAOD::Muon& m) { return std::sqrt(m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::z0,Trk::z0)); }
  static float      dd0(const xAOD::Muon& m) { return std::sqrt(m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::d0,Trk::d0)); }
  
  static float dpt(const xAOD::Muon& m) {
    double th  = theta(m);
    double dth = dtheta(m);
    double sintheta = std::sin(th);
    double costheta = std::cos(th);
    double p_ = p(m);
    double dqovp = dqOverP(m);
    double dpt2 = (p_*p_*sintheta)*(p_*p_*sintheta)*dqovp*dqovp + (p_*costheta)*(p_*costheta)*dth*dth - 2*(p_*p_*sintheta)*(p_*costheta)*covthetaOvP(m);    /// ??? <<-- check this
    if ( dpt2>0 ) return std::sqrt( dpt2 );
    return 0;
  }

  static float covthetaOvP(const xAOD::Muon& m) { return m.trackParticle(muontype())->definingParametersCovMatrix()(Trk::qOverP,Trk::theta); }; /// a covariance *not* an error

  static float charge(const xAOD::Muon& m) { return m.charge(); }
  
  static float chi2(const xAOD::Muon& m) { return m.trackParticle(muontype())->chiSquared(); }
  static float ndof(const xAOD::Muon& m) { return m.trackParticle(muontype())->numberDoF();  }  /// not an integer ???
  
  static uint8_t hasValidTime(const xAOD::Muon& m) { return m.trackParticle(muontype())->hasValidTime(); }
  static float           time(const xAOD::Muon& m) { return m.trackParticle(muontype())->time(); }

  /// do we want to keep these ??
  //  static float      pTsig(const xAOD::Muon& t) { return t.charge() ? std::copysign( t.pt(), t.charge() ) : 0.; }
  //  static float z0SinTheta(const xAOD::Muon& t) { return z0( p ) * std::sin( theta( p ) ); }
  
  static int       nPixels( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfPixelHits );       }  
  static int nPixelsShared( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfPixelSharedHits ); }
  static int   nPixelHoles( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfPixelHoles );      }
  static int  nPixelsInner( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfInnermostPixelLayerHits ); }
  static int nPixelsInnerEndcap( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfInnermostPixelLayerEndcapHits ); }

  static int       nPixelsInnerShared( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfInnermostPixelLayerSharedHits ); }
  static int nPixelsInnerSharedEndcap( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfInnermostPixelLayerSharedEndcapHits ); }
  
  static int       nPixelsBlayer( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfNextToInnermostPixelLayerHits ); }
  static int nPixelsBlayerEndcap( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfNextToInnermostPixelLayerEndcapHits ); }

    
  static int       nSCT( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfSCTHits ); }
  static int nSCTShared( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfSCTSharedHits ); }
  static int  nSCTHoles( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfSCTHoles ); }

  static int      nSi( const xAOD::Muon& m ) { return nPixels(m)+nSCT(m); }
  static int nSiHoles( const xAOD::Muon& m ) { return nPixelHoles(m)+nSCTHoles(m); } 
  
  static int   nTRT( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfTRTHits ); }
  static int nTRTHi( const xAOD::Muon& m ) { return summary_value( m, xAOD::numberOfTRTHighThresholdHitsTotal ); }
  
  template<typename U>
  static void    addChildren( const xAOD::Muon& m, std::vector<U>& v ) { v.emplace_back( m.trackParticle(muontype()) ); }
 
};




#endif  // IDTPM2_MUONTRAITS_H 







