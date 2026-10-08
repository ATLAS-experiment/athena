// emacs: this is -*- c++ -*-
/**
 **   @file    TrackTraits.h        
 **                   
 **   @author  sutt
 **   @date    Wed 11 Mar 2026 17:54:21 GMT
 **
 **   $Id: TrackTraitsA.h, v0.0   Wed 11 Mar 2026 17:54:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRACKTRAITS_H
#define  TRACKTRAITS_H

#include <cstdint>
#include <cmath>
#include <vector>
#include <stdexcept>


/// raw track traits - use for the TrackParticle by default, specialise
/// for anything else, although in principle, all the TrackParticle Specific
/// traits and the TruthParticle specific traits, probably should not
/// be added, as any tool that uses such a trait should operate *only*
/// on that type already, so the traits wrapper should not really be needed


/// NB: Note that the addChildren template must only be implemented for a
///     Traits class that has children, after the child type has been
///     completely specified


/// autodetect if T has a pt() object -- something for another day ...  
/// template<typename T>
/// using has_pt = decltype(std::declval<const T&>().pt());

/// template<typename T>
/// using has_summary =
///   decltype(std::declval<const T&>().summaryValue( std::declval<uint8_t&>(),
/// 						  std::declval<xAOD::SummaryType>() ) );




template<typename T>
struct TrackTraitsBase {
  
  static float     pt(const T& t) { return t.pt();    }
  static float    eta(const T& t) { return t.eta();   }
  static float  theta(const T&  ) { return 0; }
  static float    phi(const T& t) { return t.phi();   }
  static float     z0(const T&  ) { return 0; }
  static float     d0(const T&  ) { return 0; }
  static float qOverP(const T&  ) { return 0; }
  static float      p(const T&  ) { return 0; }

  static float      m(const T&  ) { return 0; }


  static float      E(const T&  ) { return 0; } // t.e(); }
  static float     ET(const T&  ) { return 0; }

  static float     dphi(const T& ) { return 0; }
  static float   dtheta(const T& ) { return 0; }
  static float  dqOverP(const T& ) { return 0; }
  
  static float     deta(const T& ) { return 0; }

  static float      dz0(const T& ) { return 0; }
  static float      dd0(const T& ) { return 0; }
  
  static float      dpt(const T& ) { return 0; }
  
  static float covthetaOvP(const T& ) { return 0; }
  
  static unsigned  author(const T& ) { return 0; }

  static float    chi2(const T& ) { return 0; }
  static float    ndof(const T& ) { return 0; }
  
  static uint8_t hasValidTime(const T& ) { return 0; }
  static float           time(const T& ) { return 0; }

  // do we want these ? perhaps we will need them ...
  //  static float      pTsig(const T& ) { return 0; }
  //  static float z0SinTheta(const T& ) { return 0; }

  static int             nPixels( const T& ) { return 0; }
  static int       nPixelsShared( const T& ) { return 0; }  
  static int         nPixelHoles( const T& ) { return 0; }
  
  static int        nPixelsInner( const T& ) { return 0; }
  static int  nPixelsInnerEndcap( const T& ) { return 0; }
  
  static int       nPixelsInnerShared( const T& ) { return 0; }  
  static int nPixelsInnerSharedEndcap( const T& ) { return 0; }
  
  static int       nPixelsBlayer( const T& ) { return 0; }
  static int nPixelsBlayerEndcap( const T& ) { return 0; }
  
  static int       nSCT( const T& ) { return 0; }
  static int nSCTShared( const T& ) { return 0; }
  static int  nSCTHoles( const T& ) { return 0; }

  static int        nSi( const T& ) { return 0; }
  static int   nSiHoles( const T& ) { return 0; }
  
  static int       nTRT( const T& ) { return 0; } 
  static int     nTRTHi( const T& ) { return 0; }

  
  /// parent EDM functionality for classes with children - the class U,
  /// being used for the children has to be fully specified *before* 
  /// trying to instantiate any instaces of the corresponding derived
  /// traits
  template<typename U>
  static void    addChildren( const T& , std::vector<U>& ) { }

};

template<typename T>
struct TrackTraits : public TrackTraitsBase<T> { };

template<typename T>
struct TrackTraits<const T> : TrackTraits<T> { };



#endif  // TRACKTRAITS_H 







