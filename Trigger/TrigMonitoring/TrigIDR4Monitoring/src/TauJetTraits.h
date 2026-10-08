// emacs: this is -*- c++ -*-
/**
 **   @file    TauJetTraits.h        
 **                   
 **   @author  sutt
 **   @date    Thursday 12 Mar 2026 12:10:21 GMT
 **
 **   $Id: TauJetTraits.h, v0.0   Thu 12 Mar 2026 12:10:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef  IDTPM2_TAUJETTRAITS_H
#define  IDTPM2_TAUJETTRAITS_H

#include "xAODTau/TauJetContainer.h"

#include "TrackTraits.h"

#include <cmath>
#include <iostream>

/// This is coming from the TauJet, which is not so clean a class - the "tau" candidate 
/// does not even have a z0, or d0 value - any combination of tracks that has a 4-vector, 
/// can have a z0 and a d0 value, the TauJet class is somewhat messy and difficult to 
/// actually use like an actual "particle", so we aren't really going to bother

template<>
struct TrackTraits<xAOD::TauJet> : public TrackTraitsBase<xAOD::TauJet> {

private:
  
  /// the TauTrack may not always have an valid ElementLink but the TauTrack code does not 
  /// properly check whether the link is valid  before dereferencing the said link, so we 
  /// have to duplicate some TauJet internal functionality here, so we can make that
  /// required check ourselves ... 
  inline static const SG::ConstAccessor<xAOD::TauTrack_v1::TrackParticleLinks_t> m_trackAcc = {"trackLinks"};
  
public:
  
  static float    eta(const xAOD::TauJet& t) { return t.eta();    } 
  static float    phi(const xAOD::TauJet& t) { return t.phi();    } 

  /// is tau pt signed ??? I wish we had a consistent convention
  /// we may need to force all pt to be signed
  static float     pt(const xAOD::TauJet& t) { return t.pt(); }

  static float  theta(const xAOD::TauJet& t) { return 2*std::atan(std::exp(-t.eta()));  }
  static float qOverP(const xAOD::TauJet& t) { return t.charge()*std::sin(theta(t))/std::fabs(t.pt()); }
  
  static float      E(const xAOD::TauJet& t) { return t.e(); }

  static float     ET(const xAOD::TauJet& t) { return t.e()*std::sin(theta(t)); }

  static float      m(const xAOD::TauJet& t) { return t.m(); }

  static float      p(const xAOD::TauJet& t) {
    double th = theta(t);
    if ( th==0 ) throw std::runtime_error( "divide by 0 - tau theta is 0" );
    return t.pt()/std::sin(th);
  }

  static float charge(const xAOD::TauJet& t) { return t.charge(); }

  /// talk about insanity ,... 
  template<typename U>
  static void addChildren( const xAOD::TauJet& t, std::vector<U>& v) {
    auto tt = t.tracks();
    for ( size_t i=0 ; i<tt.size() ; i++ ) {
      const auto& links = m_trackAcc(*tt[i]);
      if ( !links.empty() && links[0].isValid() ) v.emplace_back(tt[i]->track());
      /// the issue with the invalid links causing this to be necessary this has been
      /// reported, (ATLASRECTS-8445) so no point writing this
      /// error out here we know what to do about it
      //  else std::cerr << "duff TauTrack ElementLink for: " << tt[i] << std::endl;
    }
  }
  
};




#endif  // IDTPM2_TAUJETTRAITS_H 







