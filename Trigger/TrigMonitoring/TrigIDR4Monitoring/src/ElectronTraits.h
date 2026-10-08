// emacs: this is -*- c++ -*-
/**
 **   @file    ElectronTraits.h        
 **                   
 **   @author  sutt
 **
 **   @date    Thursday 12 Mar 2026 12:10:21 GMT
 **
 **   $Id: ElectronTraits.h, v0.0   Thu 12 Mar 2026 12:10:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef  IDTPM2_ELECTRONTRAITS_H
#define  IDTPM2_ELECTRONTRAITS_H

#include "xAODEgamma/ElectronContainer.h"

#include "TrackTraits.h"


template<>
struct TrackTraits<xAOD::Electron> : public TrackTraitsBase<xAOD::Electron> {
  
public:
  
  static uint8_t summary_value( const xAOD::Electron& e, xAOD::SummaryType type ) {
    uint8_t n(0); e.trackParticle()->summaryValue(n, type); return n;
  }
  
  static float     pt(const xAOD::Electron& e) { return e.pt(); }
  static float    eta(const xAOD::Electron& e) { return e.eta();    } 
  static float  theta(const xAOD::Electron& e) { return 2*std::atan(std::exp(-e.eta()));  }
  static float    phi(const xAOD::Electron& e) { return e.phi(); }

  static float      m(const xAOD::Electron& e) { return e.m(); }

  static float     z0(const xAOD::Electron& e) { return e.trackParticle()->z0() + e.trackParticle()->vz(); }
  static float     d0(const xAOD::Electron& e) { return e.trackParticle()->d0(); }

  static float qOverP(const xAOD::Electron& e) { return e.trackParticle()->qOverP(); }

  /// is this really the best way to do it ?? or should we more properly use
  /// the electron parameters themselves ?? 
  static float      p(const xAOD::Electron& e) { 
    if ( e.trackParticle()->qOverP()==0 ) throw std::runtime_error( "divide by 0 - track qOverP() is 0" );
    return 1/e.trackParticle()->qOverP();
  }

  static float      E(const xAOD::Electron& e) { return e.e(); }

  static float     ET(const xAOD::Electron& e) { return e.e()*std::sin(theta(e)); }


#if 0
  /// unsure whether we actually want the electron to report the uncertainties 
  /// of it's track as it's own here, there are pros and cons, so leave this
  /// here until we decide one way of the other 
  static float     dphi(const xAOD::Electron& e) { return std::sqrt(e.trackParticle()->definingParametersCovMatrix()(Trk::phi0,Trk::phi0)); }
  static float   dtheta(const xAOD::Electron& e) { return std::sqrt(e.trackParticle()->definingParametersCovMatrix()(Trk::theta,Trk::theta)); }
  static float  dqOverP(const xAOD::Electron& e) { return std::sqrt(e.trackParticle()->definingParametersCovMatrix()(Trk::qOverP,Trk::qOverP)); }
  
  static float     deta(const xAOD::Electron& e) { return 0.5*dtheta(e)/(std::cos(0.5*theta(e))*std::cos(0.5*theta(e))*std::tan(0.5*theta(e))); }  // ???? CHECK THIS <<--

  static float      dz0(const xAOD::Electron& e) { return std::sqrt(e.trackParticle()->definingParametersCovMatrix()(Trk::z0,Trk::z0)); }
  static float      dd0(const xAOD::Electron& e) { return std::sqrt(e.trackParticle()->definingParametersCovMatrix()(Trk::d0,Trk::d0)); }
  
  static float dpt(const xAOD::Electron& e) {
    double th  = theta(e);
    double dth = dtheta(e);
    double sintheta = std::sin(th);
    double costheta = std::cos(th);
    double p_ = p(e);
    double dqovp = dqOverP(e);
    double dpt2 = (p_*p_*sintheta)*(p_*p_*sintheta)*dqovp*dqovp + (p_*costheta)*(p_*costheta)*dth*dth - 2*(p_*p_*sintheta)*(p_*costheta)*covthetaOvP(e);    /// ??? <<-- check this
    if ( dpt2>0 ) return std::sqrt( dpt2 );
    return 0;
  }
  
  static float covthetaOvP(const xAOD::Electron& e) { return e.trackParticle()->definingParametersCovMatrix()(Trk::qOverP,Trk::theta); }; /// a covariance *not* an error
  
  static float chi2(const xAOD::Electron& e) { return e.trackParticle()->chiSquared(); }
  static float ndof(const xAOD::Electron& e) { return e.trackParticle()->numberDoF();  }  /// not an integer ???
  
  static uint8_t hasValidTime(const xAOD::Electron& e) { return e.trackParticle()->hasValidTime(); }
  static float           time(const xAOD::Electron& e) { return e.trackParticle()->time(); }

  //  static float      pTsig(const xAOD::Electron& t) { return t.charge() ? std::copysign( t.pt(), t.charge() ) : 0.; }
  //  static float z0SinTheta(const xAOD::Electron& t) { return z0( p ) * std::sin( theta( p ) ); }
  
#endif  
  
  template<typename U>
  static void    addChildren( const xAOD::Electron& e, std::vector<U>& v ) { v.emplace_back( e.trackParticle() ); }
  
};




#endif  // IDTPM2_ELECTRONTRAITS_H 







