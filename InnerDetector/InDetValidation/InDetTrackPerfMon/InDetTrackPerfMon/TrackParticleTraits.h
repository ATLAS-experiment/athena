// emacs: this is -*- c++ -*-
/**
 **   @file    TrackParticleTraits.h        
 **                   
 **   @author  sutt
 **   @date    Thursday 12 Mar 2026 12:10:21 GMT
 **
 **   $Id: TrackTraitsA.h, v0.0   Thu 12 Mar 2026 12:10:21 GMT sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **
 **/

#ifndef  TRACKPARTICLETRAITS_H
#define  TRACKPARTICLETRAITS_H

#include "InDetTrackPerfMon/TrackTraits.h"

#include "xAODTracking/TrackParticleContainer.h"


template<>
struct TrackTraits<xAOD::TrackParticle> : public TrackTraitsBase<xAOD::TrackParticle> {
  
public:
  
  static uint8_t summary_value( const xAOD::TrackParticle& t, xAOD::SummaryType type ) {
    uint8_t n(0); t.summaryValue(n, type); return n;
  }

  
  static float     z0(const xAOD::TrackParticle& t) { return t.z0()+t.vz();    }

  /// given the complexity of these, perhaps the TruthParticle should be the
  /// default template implementation, and these the specialisations
  static float     dphi(const xAOD::TrackParticle& t) { return std::sqrt(t.definingParametersCovMatrix()(Trk::phi0,Trk::phi0)); }
  static float   dtheta(const xAOD::TrackParticle& t) { return std::sqrt(t.definingParametersCovMatrix()(Trk::theta,Trk::theta)); }
  static float  dqOverP(const xAOD::TrackParticle& t) { return std::sqrt(t.definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP)); }
  
  static float     deta(const xAOD::TrackParticle& t) { return 0.5*dtheta(t)/(std::cos(0.5*theta(t))*std::cos(0.5*theta(t))*std::tan(0.5*theta(t))); }  // ???? CHECK THIS <<--

  static float      dz0(const xAOD::TrackParticle& t) { return std::sqrt(t.definingParametersCovMatrix()(Trk::z0,Trk::z0)); }
  static float      dd0(const xAOD::TrackParticle& t) { return std::sqrt(t.definingParametersCovMatrix()(Trk::d0,Trk::d0)); }
  
  static float dpt(const xAOD::TrackParticle& t) {
    double th  = theta(t);
    double dth = dtheta(t);
    double sintheta = std::sin(th);
    double costheta = std::cos(th);
    double p_ = p(t);
    double dqovp = dqOverP(t);
    double dpt2 = (p_*p_*sintheta)*(p_*p_*sintheta)*dqovp*dqovp + (p_*costheta)*(p_*costheta)*dth*dth - 2*(p_*p_*sintheta)*(p_*costheta)*covthetaOvP(t);    /// ??? <<-- check this
    if ( dpt2>0 ) return std::sqrt( dpt2 );
    return 0;
  }
  
  static float covthetaOvP(const xAOD::TrackParticle& t) { return t.definingParametersCovMatrix()(Trk::qOverP,Trk::theta); }; /// a covariance *not* an error
  
  static float chi2(const xAOD::TrackParticle& t) { return t.chiSquared(); }
  static float ndof(const xAOD::TrackParticle& t) { return t.numberDoF();  }  /// not an integer ???
  
  static uint8_t hasValidTime(const xAOD::TrackParticle& t) { return t.hasValidTime(); }
  static float           time(const xAOD::TrackParticle& t) { return t.time(); }

  //  static float      pTsig(const xAOD::TrackParticle& t) { return t.charge() ? std::copysign( t.pt(), t.charge() ) : 0.; }
  //  static float z0SinTheta(const xAOD::TrackParticle& t) { return z0( p ) * std::sin( theta( p ) ); }
  
  /// all these summary_value() quantities are ridiculous, you largey could
  /// achieve the same with a single bitmap without all these ludicrously
  /// long enumeration labels

  /// incidentally, since these are all uint8_t, why don't we just
  /// return uint8_t ? Why this messing about impllicitly converting
  /// to int (or event float in the IDTPM helper code

  static int       nPixels( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfPixelHits );       }  
  static int nPixelsShared( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfPixelSharedHits ); }
  static int   nPixelHoles( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfPixelHoles );      }

  static int       nPixelsInner( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfInnermostPixelLayerHits ); }
  static int nPixelsInnerEndcap( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfInnermostPixelLayerEndcapHits ); }

  static int       nPixelsInnerShared( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfInnermostPixelLayerSharedHits ); }
  static int nPixelsInnerSharedEndcap( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfInnermostPixelLayerSharedEndcapHits ); }
  
  static int       nPixelsBlayer( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfNextToInnermostPixelLayerHits ); }
  static int nPixelsBlayerEndcap( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfNextToInnermostPixelLayerEndcapHits ); }

  ///  static int nPixelHolesEndcap( const xAOD::TrackParticle& ) { 
  ///   uint8_t hits = 0;
  ///   return t.summary_value( hits, xAOD::numberOfInnermostPixelLayerHits );
  ///  }
    
  static int       nSCT( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfSCTHits ); }
  static int nSCTShared( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfSCTSharedHits ); }
  static int  nSCTHoles( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfSCTHoles ); }

  static int      nSi( const xAOD::TrackParticle& t ) { return nPixels(t)+nSCT(t); }
  static int nSiHoles( const xAOD::TrackParticle& t ) { return nPixelHoles(t)+nSCTHoles(t); } 
  
  static int   nTRT( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfTRTHits ); }
  static int nTRTHi( const xAOD::TrackParticle& t ) { return summary_value( t, xAOD::numberOfTRTHighThresholdHitsTotal ); }

  //    if constexpr (std::is_invocable_v<has_summary<T>>) {
  //      uint8_t hits = 0;
  //      t.summaryValue( hits, xAOD::numberOfInnermostPixelLayerHits );
  //      return hits;
  //    }
  //    else return -1;
  //  }
  
};




#endif  // TRACKPARTICLETRAITS_H 







