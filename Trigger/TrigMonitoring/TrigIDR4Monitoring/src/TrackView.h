/* emacs: this is -*- c++ -*- */
/**
 **   @file    TrackView.h        
 **                   
 **   @author  sutt
 **   @date    Fri 13 Mar 2026 23:01:14 GMT
 **
 **   $Id: TrackView.h, v0.0   Sat 14 Mar 2026 23:01:14 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRIGIDR4MONITORING_TRACKVIEW_H
#define  TRIGIDR4MONITORING_TRACKVIEW_H

#include "TrackTraits.h"

#include <vector>
#include <cstdint>
#include <iostream>
#include <typeinfo>

///
/// Type-erased view on a generic track class - implement a TrackTraits<T>
/// to use this for any other type of class
///

/// to check that these are not actually being constructed
/// with the wrong classes, we can include the static_asserts
/// in the constructor to check the types, but then need the
/// actual type headers which is a bit unfortunate
/// these are useful, so keep these includes in case we need them
/// later

// #include "xAODTracking/TrackParticle.h"
// #include "xAODTruth/TruthParticle.h"

class ChildView;  /// forward declaration required for function prototype


class TrackView {

public:

  template<typename T>
  TrackView(const T* pobj) : m_ptr(pobj), m_vtable(&table<T>) {
#if 0
    /// rather than test for these specific types, it would be
    /// better to check for the generic input type T, that there
    /// really is a TrackTraits<T> specialisation at compile
    /// time, or else fail, but am still trying to get that
    /// to work simply
    using BareT = std::remove_cv_t<T>;
    static_assert( std::is_same_v<BareT, xAOD::TrackParticle> || std::is_same_v<BareT, xAOD::TruthParticle>,
		   "unsupported TrackView type" );
#endif
  }

  template<typename T,
	   typename=std::enable_if_t< !std::is_pointer_v<T> && !std::is_same_v<std::decay_t<T>, TrackView> > > 
  TrackView(const T& obj) : m_ptr(&obj), m_vtable(&table<T>) {
#if 0
    /// as above ...
    using BareT = std::remove_cv_t<T>;
    static_assert( std::is_same_v<BareT, xAOD::TrackParticle> || std::is_same_v<BareT, xAOD::TruthParticle>,
		   "unsupported TrackView type" );
#endif		  
  }

  TrackView( const TrackView& t ) = default;

  TrackView( const TrackView* ) = delete; // prevent implicitly creating a TrackView with an internal TrackView*
  TrackView(       TrackView* ) = delete;

  virtual ~TrackView() = default;


  const void* ptr() const { return m_ptr; } 
  
  float     pt() const { return m_vtable->pt(m_ptr);     }
  float    eta() const { return m_vtable->eta(m_ptr);    }
  float  theta() const { return m_vtable->theta(m_ptr);  }

  virtual float    phi() const { return m_vtable->phi(m_ptr);    }
  virtual float     z0() const { return m_vtable->z0(m_ptr);     }
  virtual float     d0() const { return m_vtable->d0(m_ptr);     }

  float      m() const { return m_vtable->m(m_ptr);     }

  float qOverP() const { return m_vtable->qOverP(m_ptr); }

  float       p() const { return m_vtable->p(m_ptr);       }
  float       E() const { return m_vtable->E(m_ptr);       }
  float      ET() const { return m_vtable->ET(m_ptr);      }
  float    dphi() const { return m_vtable->dphi(m_ptr);    }
  float  dtheta() const { return m_vtable->dtheta(m_ptr);  }
  float dqOverP() const { return m_vtable->dqOverP(m_ptr); }
  float    deta() const { return m_vtable->deta(m_ptr);    }
  float     dz0() const { return m_vtable->dz0(m_ptr);     }
  float     dd0() const { return m_vtable->dd0(m_ptr);     }
  float     dpt() const { return m_vtable->dpt(m_ptr);     }

  float  covthetaOvP() const { return m_vtable->covthetaOvP(m_ptr); }

  unsigned  author() const { return m_vtable->author(m_ptr);}

  float       chi2() const { return m_vtable->chi2(m_ptr);  }
  float       ndof() const { return m_vtable->ndof(m_ptr);  }

  uint8_t  hasValidTime() const { return m_vtable->hasValidTime(m_ptr); }
  float            time() const { return m_vtable->time(m_ptr);         }

  int           nPixels() const { return m_vtable->nPixels(m_ptr);       }
  int     nPixelsShared() const { return m_vtable->nPixelsShared(m_ptr); }
  int       nPixelHoles() const { return m_vtable->nPixelHoles(m_ptr);   }

  int        nPixelsInner() const { return m_vtable->nPixelsInner(m_ptr);       }
  int  nPixelsInnerEndcap() const { return m_vtable->nPixelsInnerEndcap(m_ptr); }

  int        nPixelsInnerShared() const { return m_vtable->nPixelsInnerShared(m_ptr);       }
  int  nPixelsInnerSharedEndcap() const { return m_vtable->nPixelsInnerSharedEndcap(m_ptr); }

  int        nPixelsBlayer() const { return m_vtable->nPixelsBlayer(m_ptr);       }
  int  nPixelsBlayerEndcap() const { return m_vtable->nPixelsBlayerEndcap(m_ptr); }

  int          nSCT() const { return m_vtable->nSCT(m_ptr);       }
  int    nSCTShared() const { return m_vtable->nSCTShared(m_ptr); }
  int     nSCTHoles() const { return m_vtable->nSCTHoles(m_ptr);  }

  int           nSi() const { return m_vtable->nSi(m_ptr);      }
  int      nSiHoles() const { return m_vtable->nSiHoles(m_ptr); }

  int          nTRT() const { return m_vtable->nTRT(m_ptr);     }
  int        nTRTHi() const { return m_vtable->nTRTHi(m_ptr);   }

  const std::type_info& type() const { return *m_vtable->m_type; }

  template<typename T> bool is() const { return type()==typeid(T); }

  template<typename T> const T* as() const { return is<T>() ? static_cast<const T*>(m_ptr) : 0; }


  /// standard TrackView can't have any children 

  virtual bool isComposite() const { return false; }

  virtual bool hasChildren() const { return false; }
  
  virtual size_t childCount() const { return 0; }

  virtual const	TrackView* child(size_t ) const { return 0; }

  /// and doesn't have a parent ...
  
  virtual bool hasParent() const { return false; }

  virtual const TrackView* parent() const { return 0; }

  virtual bool hasDecorations() const { return false; }

  /// don't want a standard TrackView to have children 
  /// void addChildren( std::vector<TrackView>& v ); 

private:
  
  struct VTable;
    
protected:

  const VTable*  vtable() const { return m_vtable; };
  
private:

  const void*    m_ptr;
  const VTable*  m_vtable;

private:

  /// so we can more easily change the type if required ...

  using CHILDTYPE = ChildView;

  struct VTable {
    float      (*pt)(const void*);
    float     (*eta)(const void*);
    float   (*theta)(const void*);
    float     (*phi)(const void*);
    float      (*z0)(const void*);
    float      (*d0)(const void*);
    float       (*m)(const void*);
    float  (*qOverP)(const void*);
    float       (*p)(const void*);
    float       (*E)(const void*);
    float      (*ET)(const void*);
    float    (*dphi)(const void*);
    float  (*dtheta)(const void*);
    float (*dqOverP)(const void*);
    float    (*deta)(const void*);
    float     (*dz0)(const void*);
    float     (*dd0)(const void*);
    float     (*dpt)(const void*);

    float (*covthetaOvP)(const void*);

    unsigned      (*author)(const void*);

    float           (*chi2)(const void*);
    float           (*ndof)(const void*);

    uint8_t (*hasValidTime)(const void*);
    float           (*time)(const void*);

    int          (*nPixels)(const void*);
    int    (*nPixelsShared)(const void*);
    int      (*nPixelHoles)(const void*);

    int        (*nPixelsInner)(const void*);
    int  (*nPixelsInnerEndcap)(const void*);

    int        (*nPixelsInnerShared)(const void*);
    int  (*nPixelsInnerSharedEndcap)(const void*);

    int        (*nPixelsBlayer)(const void*);
    int  (*nPixelsBlayerEndcap)(const void*);

    int         (*nSCT)(const void*);
    int   (*nSCTShared)(const void*);
    int    (*nSCTHoles)(const void*);
    int          (*nSi)(const void*);
    int     (*nSiHoles)(const void*);
    int         (*nTRT)(const void*);
    int       (*nTRTHi)(const void*);

    /// need this in the vtable, but we don't want it exposed in the TrackView
    void    (*addChildren)(const void*, std::vector<CHILDTYPE>& );

    const std::type_info* m_type; 
  };
    
  template<typename T>  static const VTable table;

  
  template<typename T>
  static const T& deref(const void* p) { return *static_cast<const T*>(p); }
  
  template<typename T>  static float       pt_impl(const void* p) { return TrackTraits<T>::pt(deref<T>(p));     }
  template<typename T>  static float      eta_impl(const void* p) { return TrackTraits<T>::eta(deref<T>(p));    }
  template<typename T>  static float    theta_impl(const void* p) { return TrackTraits<T>::theta(deref<T>(p));  }
  template<typename T>  static float      phi_impl(const void* p) { return TrackTraits<T>::phi(deref<T>(p));    }
  template<typename T>  static float       z0_impl(const void* p) { return TrackTraits<T>::z0(deref<T>(p));     }
  template<typename T>  static float       d0_impl(const void* p) { return TrackTraits<T>::d0(deref<T>(p));     }
  template<typename T>  static float        m_impl(const void* p) { return TrackTraits<T>::m(deref<T>(p));      }
  template<typename T>  static float   qOverP_impl(const void* p) { return TrackTraits<T>::qOverP(deref<T>(p)); }

  template<typename T>  static float        p_impl(const void* p) { return TrackTraits<T>::p(deref<T>(p));    }
  template<typename T>  static float        E_impl(const void* p) { return TrackTraits<T>::E(deref<T>(p));    }
  template<typename T>  static float       ET_impl(const void* p) { return TrackTraits<T>::ET(deref<T>(p));   }
  template<typename T>  static float     dphi_impl(const void* p) { return TrackTraits<T>::dphi(deref<T>(p)); }
  template<typename T>  static float   dtheta_impl(const void* p) { return TrackTraits<T>::dtheta(deref<T>(p)); }
  template<typename T>  static float  dqOverP_impl(const void* p) { return TrackTraits<T>::dqOverP(deref<T>(p)); }
  template<typename T>  static float     deta_impl(const void* p) { return TrackTraits<T>::deta(deref<T>(p)); }
  template<typename T>  static float      dz0_impl(const void* p) { return TrackTraits<T>::dz0(deref<T>(p));  }
  template<typename T>  static float      dd0_impl(const void* p) { return TrackTraits<T>::dd0(deref<T>(p));  }
  template<typename T>  static float      dpt_impl(const void* p) { return TrackTraits<T>::dpt(deref<T>(p));  }
  template<typename T>  static float  covthetaOvP_impl(const void* p) { return TrackTraits<T>::covthetaOvP(deref<T>(p)); }
  template<typename T>  static unsigned    author_impl(const void* p) { return TrackTraits<T>::author(deref<T>(p)); }
  template<typename T>  static float         chi2_impl(const void* p) { return TrackTraits<T>::chi2(deref<T>(p));   }
  template<typename T>  static float         ndof_impl(const void* p) { return TrackTraits<T>::ndof(deref<T>(p));   }
  template<typename T>  static uint8_t  hasValidTime_impl(const void* p) { return TrackTraits<T>::hasValidTime(deref<T>(p)); }
  template<typename T>  static float            time_impl(const void* p) { return TrackTraits<T>::time(deref<T>(p));         }
  template<typename T>  static int              nPixels_impl(const void* p) { return TrackTraits<T>::nPixels(deref<T>(p));       }
  template<typename T>  static int        nPixelsShared_impl(const void* p) { return TrackTraits<T>::nPixelsShared(deref<T>(p)); }
  template<typename T>  static int          nPixelHoles_impl(const void* p) { return TrackTraits<T>::nPixelHoles(deref<T>(p));   }
  template<typename T>  static int         nPixelsInner_impl(const void* p) { return TrackTraits<T>::nPixelsInner(deref<T>(p));       }
  template<typename T>  static int   nPixelsInnerEndcap_impl(const void* p) { return TrackTraits<T>::nPixelsInnerEndcap(deref<T>(p)); }
  template<typename T>  static int         nPixelsInnerShared_impl(const void* p) { return TrackTraits<T>::nPixelsInnerShared(deref<T>(p));       }
  template<typename T>  static int   nPixelsInnerSharedEndcap_impl(const void* p) { return TrackTraits<T>::nPixelsInnerSharedEndcap(deref<T>(p)); }
  template<typename T>  static int         nPixelsBlayer_impl(const void* p) { return TrackTraits<T>::nPixelsBlayer(deref<T>(p));       }
  template<typename T>  static int   nPixelsBlayerEndcap_impl(const void* p) { return TrackTraits<T>::nPixelsBlayerEndcap(deref<T>(p)); }
  template<typename T>  static int         nSCT_impl(const void* p) { return TrackTraits<T>::nSCT(deref<T>(p));       }
  template<typename T>  static int   nSCTShared_impl(const void* p) { return TrackTraits<T>::nSCTShared(deref<T>(p)); }
  template<typename T>  static int    nSCTHoles_impl(const void* p) { return TrackTraits<T>::nSCTHoles(deref<T>(p));  }
  template<typename T>  static int          nSi_impl(const void* p) { return TrackTraits<T>::nSi(deref<T>(p));      }
  template<typename T>  static int     nSiHoles_impl(const void* p) { return TrackTraits<T>::nSiHoles(deref<T>(p)); }
  template<typename T>  static int         nTRT_impl(const void* p) { return TrackTraits<T>::nTRT(deref<T>(p));   }
  template<typename T>  static int       nTRTHi_impl(const void* p) { return TrackTraits<T>::nTRTHi(deref<T>(p)); }
  template<typename T>  static void addChildren_impl(const void* p, std::vector<CHILDTYPE>& v ) { TrackTraits<T>::template addChildren<CHILDTYPE>(deref<T>(p), v ); } 

};
  
template<typename T>
inline const TrackView::VTable TrackView::table = {
    &TrackView::pt_impl<T>,
    &TrackView::eta_impl<T>,
    &TrackView::theta_impl<T>,
    &TrackView::phi_impl<T>,
    &TrackView::z0_impl<T>,
    &TrackView::d0_impl<T>,
    &TrackView::m_impl<T>,
    &TrackView::qOverP_impl<T>,
    &TrackView::p_impl<T>,
    &TrackView::E_impl<T>,
    &TrackView::ET_impl<T>,
    &TrackView::dphi_impl<T>,
    &TrackView::dtheta_impl<T>,
    &TrackView::dqOverP_impl<T>,
    &TrackView::deta_impl<T>,
    &TrackView::dz0_impl<T>,
    &TrackView::dd0_impl<T>,
    &TrackView::dpt_impl<T>,
    &TrackView::covthetaOvP_impl<T>,
    &TrackView::author_impl<T>,
    &TrackView::chi2_impl<T>,
    &TrackView::ndof_impl<T>,
    &TrackView::hasValidTime_impl<T>,
    &TrackView::time_impl<T>,
    &TrackView::nPixels_impl<T>,
    &TrackView::nPixelsShared_impl<T>,
    &TrackView::nPixelHoles_impl<T>,
    &TrackView::nPixelsInner_impl<T>,
    &TrackView::nPixelsInnerEndcap_impl<T>,
    &TrackView::nPixelsInnerShared_impl<T>,
    &TrackView::nPixelsInnerSharedEndcap_impl<T>,
    &TrackView::nPixelsBlayer_impl<T>,
    &TrackView::nPixelsBlayerEndcap_impl<T>,
    &TrackView::nSCT_impl<T>,
    &TrackView::nSCTShared_impl<T>,
    &TrackView::nSCTHoles_impl<T>,
    &TrackView::nSi_impl<T>,
    &TrackView::nSiHoles_impl<T>,
    &TrackView::nTRT_impl<T>,
    &TrackView::nTRTHi_impl<T>,
    &TrackView::addChildren_impl<T>,
    &typeid(T)
};


inline std::ostream& operator<<( std::ostream& s, const TrackView& t ) {
  s << "  [ "
    <<   "eta=" << t.eta()
    << "\tphi=" << t.phi()
    << "\tpt="  << t.pt()
    << "\tz0="  << t.z0()
    << "\td0="  << t.d0();
  s << "\t : ptr=" << t.ptr() << " (" << &t << ")" << " ]";

  if ( t.hasParent() ) s << "\t( parent: " << t.parent()->ptr() << " )";

  if ( t.hasChildren() ) {
    for ( size_t i=0 ; i<t.childCount() ; i++ ) s << "\n\t\t[" << *t.child(i) << "]";
  }
  return s;
}



#endif  // TRIGIDR4MONITORING_TRACKVIEW_H 










