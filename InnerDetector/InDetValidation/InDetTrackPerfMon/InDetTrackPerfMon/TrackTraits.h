// emacs: this is -*- c++ -*-
/**
 **   @file    TrackTraits.h        
 **                   
 **   @author  sutt
 **   @date    Wed 11 Mar 2026 17:54:21 GMT
 **
 **   $Id: TrackTraitsA.h, v0.0   Wed 11 Mar 2026 17:54:21 GMT sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **
 **/


#ifndef  TRACKTRAITS_H
#define  TRACKTRAITS_H

#include <cstdint>
#include <cmath>
#include <stdexcept>


/// raw track traits - use for the TrackParticle by default, specialise
/// for anything else, although in principle, all the TrackParticle Specific
/// traits and the TruthParticle specific traits, probably should not
/// be added, as any tool that uses such a trait should operate *only*
/// on that type already, so the traits wrapper should not really be needed

/// NB: note that if we moved the template declaration to each mothod of the
///     traits then we would have automatic overloading for each trait, and
///     would not need to specify the type, but then would not be able to
///     use the adapter - since the adaptor is the real purpose of doing it 
///     this way we leave it as it is


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
  static float  theta(const T& t) { return t.theta(); }
  static float    phi(const T& t) { return t.phi();   }
  static float     z0(const T& t) { return t.z0()+t.vz();    }
  static float     d0(const T& t) { return t.d0();    }
  static float qOverP(const T& t) { return t.qOverP(); }

  static float p(const T& t) {
    if ( t.qOverP()==0 ) throw std::runtime_error( "possible corrupted track - this should never happen" );
    double p_ = 1/t.qOverP();
    return p_;
  }

  static float      E(const T& t) { return t.e(); }
  static float     ET(const T& t) { return t.e()*std::sin(t.theta()); }

  /// given the complexity of these, perhaps the TruthParticle should be the
  /// default template implementation, and these the specialisations
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

};

template<typename T>
struct TrackTraits : public TrackTraitsBase<T> { };

template<typename T>
struct TrackTraits<const T> : TrackTraits<T> { };


class ITrackAdaptor {
public:

  virtual float     pt() const = 0;
  virtual float    eta() const = 0;
  virtual float  theta() const = 0;
  virtual float    phi() const = 0; 
  virtual float     d0() const = 0; 
  virtual float     z0() const = 0;
  virtual float qOverP() const = 0;

  virtual float      p() const = 0;
    
  virtual float      E() const = 0;
  virtual float     ET() const = 0;

  virtual unsigned  author() const = 0;

  virtual float    dphi() const = 0;
  virtual float  dtheta() const = 0;
  virtual float    deta() const = 0;
  virtual float     dz0() const = 0;
  virtual float     dd0() const = 0;
  virtual float     dpt() const = 0;

  virtual float     dqOverP() const = 0;
  virtual float covthetaOvP() const = 0;
  
  /// techniocally these should not be included:
  /// any code which depende on these must be for Tracks rather than truth
  /// particles, so could use the track classes directly
  /// but put tyhem here in case we want other track classes
  virtual float chi2() const = 0;
  virtual float ndof() const = 0;

  virtual uint8_t hasValidTime() const = 0;
  virtual float           time() const = 0;

  
  /// spacepoints - these shouldn't be here really these are helpers,
  /// not part of an adaptor
  virtual int             nPixels() const = 0;
  virtual int       nPixelsShared() const = 0;
  virtual int         nPixelHoles() const = 0;


  virtual int        nPixelsInner() const = 0;
  virtual int  nPixelsInnerEndcap() const = 0;

  virtual int       nPixelsInnerShared() const = 0;
  virtual int nPixelsInnerSharedEndcap() const = 0;

  virtual int       nPixelsBlayer() const = 0;
  virtual int nPixelsBlayerEndcap() const = 0;

  virtual int       nSCT() const = 0;
  virtual int nSCTShared() const = 0;
  virtual int  nSCTHoles() const = 0;

  virtual int        nSi() const = 0;
  virtual int   nSiHoles() const = 0;
  
  virtual int       nTRT() const = 0;
  virtual int     nTRTHi() const = 0;
  
};





// adaptor class - uses the traits

template<typename T>
class TrackAdaptor : public ITrackAdaptor {

  using Traits = TrackTraits<std::remove_cvref_t<T>>;

public:
  
  TrackAdaptor( T& t ) : m_t(&t) { } 
  TrackAdaptor( T* t ) : m_t(t)  { } 
  
  float     pt() const { return Traits::pt(*m_t);     }
  float    eta() const { return Traits::eta(*m_t);    }
  float  theta() const { return Traits::theta(*m_t);  }
  float    phi() const { return Traits::phi(*m_t);    }
  float     d0() const { return Traits::d0(*m_t);     }
  float     z0() const { return Traits::z0(*m_t);     }
  float qOverP() const { return Traits::qOverP(*m_t); }
  
#if 0
  float     pt() const { return TrackTraits<T>::pt(*m_t);     }
  float    eta() const { return TrackTraits<T>::eta(*m_t);    }
  float  theta() const { return TrackTraits<T>::theta(*m_t);  }
  float    phi() const { return TrackTraits<T>::phi(*m_t);    }
  float     d0() const { return TrackTraits<T>::d0(*m_t);     }
  float     z0() const { return TrackTraits<T>::z0(*m_t);     }
  float qOverP() const { return TrackTraits<T>::qOverP(*m_t); }
#endif
  
  float      p() const { return TrackTraits<T>::p(*m_t); }
  
  float      E() const { return TrackTraits<T>::E(*m_t);  }
  float     ET() const { return TrackTraits<T>::ET(*m_t); }

  float    dphi() const { return TrackTraits<T>::dphi(*m_t);    }
  float  dtheta() const { return TrackTraits<T>::dtheta(*m_t);  }
  float    deta() const { return TrackTraits<T>::deta(*m_t);    }
  float     dz0() const { return TrackTraits<T>::dz0(*m_t);     }
  float     dd0() const { return TrackTraits<T>::dd0(*m_t);     }
  float     dpt() const { return TrackTraits<T>::dpt(*m_t);     }

  float     dqOverP() const { return TrackTraits<T>::dqOverP(*m_t);     }
  float covthetaOvP() const { return TrackTraits<T>::covthetaOvP(*m_t); }
  
  unsigned  author() const { return TrackTraits<T>::author(*m_t);  }

  /// techniocally these should not be included:
  /// any code which depende on these must be for Tracks rather than truth
  /// particles, so could use the track classes directly
  /// but put tyhem here in case we want other track classes
  float chi2() const { return TrackTraits<T>::chi2(*m_t); }
  float ndof() const { return TrackTraits<T>::ndof(*m_t);       }

  uint8_t hasValidTime() const { return TrackTraits<T>::hasValidTime(*m_t); }
  float           time() const { return TrackTraits<T>::time(*m_t); }

  /// spacepoints - shouldn;t really be in this adaptor
  

  int             nPixels() const { return TrackTraits<T>::nPixels(*m_t);       }
  int       nPixelsShared() const { return TrackTraits<T>::nPixelsShared(*m_t); }
  int         nPixelHoles() const { return TrackTraits<T>::nPixelHoles(*m_t);   }

  int        nPixelsInner() const { return TrackTraits<T>::nPixelsInner(*m_t);       }
  int  nPixelsInnerEndcap() const { return TrackTraits<T>::nPixelsInnerEndcap(*m_t); }

  int       nPixelsInnerShared() const { return TrackTraits<T>::nPixelsInnerShared(*m_t);       }
  int nPixelsInnerSharedEndcap() const { return TrackTraits<T>::nPixelsInnerSharedEndcap(*m_t); }

  int       nPixelsBlayer() const{ return TrackTraits<T>::nPixelsBlayer(*m_t);       }
  int nPixelsBlayerEndcap() const { return TrackTraits<T>::nPixelsBlayerEndcap(*m_t); }
  
    
  int       nSCT() const { return TrackTraits<T>::nSCT(*m_t);       }
  int nSCTShared() const { return TrackTraits<T>::nSCTShared(*m_t); }
  int  nSCTHoles() const { return TrackTraits<T>::nSCTHoles(*m_t);  }
   
  int        nSi() const { return TrackTraits<T>::nSi(*m_t);      }
  int   nSiHoles() const { return TrackTraits<T>::nSiHoles(*m_t); }
  
  int       nTRT() const { return TrackTraits<T>::nTRT(*m_t);   }
  int     nTRTHi() const { return TrackTraits<T>::nTRTHi(*m_t); }

  
private:

  T* m_t; 
  
};

/// factor functions to create the adapators ...
template<typename T>
std::unique_ptr<ITrackAdaptor> makeAdaptor( T* t ) {
  return std::make_unique<TrackAdaptor<T> >(*t);
}


template<typename T>
std::unique_ptr<ITrackAdaptor> makeAdaptor( T& t ) {
  return std::make_unique<TrackAdaptor<T> >(t);
}




#endif  // TRACKTRAITS_H 







