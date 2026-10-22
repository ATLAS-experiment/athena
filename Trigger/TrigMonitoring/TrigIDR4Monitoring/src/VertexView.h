/* emacs: this is -*- c++ -*- */
/**
 **   @file    VertexView.h        
 **                   
 **   @author  sutt
 **   @date    Sat 20 Jun 2026 08:48:40 BST
 **
 **   $Id: VertexView.h, v0.0   Sat 20 Jun 2026 08:48:40 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRIGIDR4MONITORING_VERTEXVIEW_H
#define  TRIGIDR4MONITORING_VERTEXVIEW_H

#include <vector>
#include <cstdint>
#include <iostream>
#include <typeinfo>

#include "VertexTraits.h"

/// Type-erased view on a generic vertex class - implementation
/// won't bother with any VertexTraits at the moment, since the
/// only functions where the xAOD::TruthVertex and xAOD::Vertex
/// have any overlap are the x(), y() and z(), and they have the
/// same name, so there is literally no point at the moment
///
/// at some point we will want to extend this class so that it can
/// properly include information on which tracks lie on the vertex
/// since for proper selection, in an Roi it is not enough to verify
/// that the z range is within the Roi, we need to check that AT
/// LEAST one of the tracks on the vertex is in the Roi as well -
/// if none of the tracks on the vertex is in the Roi then the
/// vertex can not be found in that Roi - this is quite am important
/// aspect - particularly for jet and bjet vertices in high pileup
/// events


class VertexView {

public:

  template<typename T>
  VertexView(const T* pobj) : m_ptr(pobj), m_vtable(&table<T>) {
#if 0
    /// rather than test for these specific types, it would be
    /// better to check for the generic input type T, that there
    /// exist some Traits
    using BareT = std::remove_cv_t<T>;
    static_assert( std::is_same_v<BareT, xAOD::Vertex> || std::is_same_v<BareT, xAOD::TruthVertex>,
		   "unsupported VertexView type" );
#endif
  }

  template<typename T,
	   typename=std::enable_if_t< !std::is_pointer_v<T> && !std::is_same_v<std::decay_t<T>, VertexView> > > 
  VertexView(const T& obj) : m_ptr(&obj), m_vtable(&table<T>) {  }

  VertexView( const VertexView& t ) = default;

  VertexView( const VertexView* ) = delete; // don't want implicitly creating a VertexView with an internal VertexView*
  VertexView(       VertexView* ) = delete;

  virtual ~VertexView() = default;

  VertexView& operator=( const VertexView& v) = default;

  const void* ptr() const { return m_ptr; } 
  
  float     x() const { return m_vtable->x(m_ptr); }
  float     y() const { return m_vtable->y(m_ptr); }
  float     z() const { return m_vtable->z(m_ptr); }

  float    dx() const { return m_vtable->dx(m_ptr); }
  float    dy() const { return m_vtable->dy(m_ptr); }
  float    dz() const { return m_vtable->dz(m_ptr); }

  size_t  nTrackParticles() const { return m_vtable->nTrackParticles(m_ptr); }
  float        chiSquared() const { return m_vtable->chiSquared(m_ptr); }
  float         numberDoF() const { return m_vtable->numberDoF(m_ptr); }

  float              time() const { return m_vtable->time(m_ptr); }

  int          vertexType() const { return m_vtable->vertexType(m_ptr); }

  const std::type_info&       type() const { return *m_vtable->m_type; }

  template<typename T> bool     is() const { return type()==typeid(T); }

  template<typename T> const T* as() const { return is<T>() ? static_cast<const T*>(m_ptr) : 0; }

  
private:

  struct VTable;

  const void*    m_ptr;
  const VTable*  m_vtable;

private:
  
  struct VTable {
    float      (*x)(const void*);
    float      (*y)(const void*);
    float      (*z)(const void*);

    float     (*dx)(const void*);
    float     (*dy)(const void*);
    float     (*dz)(const void*);

    size_t   (*nTrackParticles)(const void*);
    float         (*chiSquared)(const void*);
    float          (*numberDoF)(const void*);

    float               (*time)(const void*);

    int           (*vertexType)(const void*);

    const std::type_info* m_type; 
  };
    
  template<typename T>  static const VTable table;

  template<typename T>
  static const T& deref(const void* p) { return *static_cast<const T*>(p); }

  template<typename T>  static float  x_impl(const void* p) { return VertexTraits<T>::x(deref<T>(p));  }
  template<typename T>  static float  y_impl(const void* p) { return VertexTraits<T>::y(deref<T>(p));  }
  template<typename T>  static float  z_impl(const void* p) { return VertexTraits<T>::z(deref<T>(p));  }

  template<typename T>  static float dx_impl(const void* p) { return VertexTraits<T>::dx(deref<T>(p));  }
  template<typename T>  static float dy_impl(const void* p) { return VertexTraits<T>::dy(deref<T>(p));  }
  template<typename T>  static float dz_impl(const void* p) { return VertexTraits<T>::dz(deref<T>(p));  }

  template<typename T>  static size_t nTrackParticles_impl(const void* p) { return VertexTraits<T>::nTrackParticles(deref<T>(p)); }
  template<typename T>  static float       chiSquared_impl(const void* p) { return VertexTraits<T>::chiSquared(deref<T>(p));      }
  template<typename T>  static float        numberDoF_impl(const void* p) { return VertexTraits<T>::numberDoF(deref<T>(p));       }

  template<typename T>  static float             time_impl(const void* p) { return VertexTraits<T>::time(deref<T>(p));       }

  template<typename T>  static int vertexType_impl(const void* p) { return VertexTraits<T>::vertexType(deref<T>(p));  }
  

};
  
template<typename T>
inline const VertexView::VTable VertexView::table = {
  &VertexView::x_impl<T>,
  &VertexView::y_impl<T>,
  &VertexView::z_impl<T>,
  &VertexView::dx_impl<T>,
  &VertexView::dy_impl<T>,
  &VertexView::dz_impl<T>,
  &VertexView::nTrackParticles_impl<T>,
  &VertexView::chiSquared_impl<T>,
  &VertexView::numberDoF_impl<T>,
  &VertexView::time_impl<T>,
  &VertexView::vertexType_impl<T>,
  &typeid(T)
};


inline std::ostream& operator<<( std::ostream& s, const VertexView& v ) {
  s << "[ x=" << v.x()
    << "\ty=" << v.y()
    << "\tz=" << v.z()
    << "\tntracks= " << v.nTrackParticles()
    << "\ttype="     << v.vertexType()
    << "\t : ptr="   << v.ptr()
    << " ]";
  return s;
}


/// annoying that they always include this "dummy" vertex ... 
inline bool dummyVertex(const VertexView& v) { return v.vertexType()==0; }


template<typename T>
std::vector<VertexView> makevertexview( const std::vector<const T*>& v ) {
  std::vector<VertexView> tv( v.begin(), v.end() );
  /// remove the "dummy" vertices
  tv.erase( std::remove_if(tv.begin(), tv.end(), dummyVertex ), tv.end());
  return tv;
}

template<typename T>
std::vector<VertexView> makevertexview( const std::vector<T*>& v ) {
  std::vector<VertexView> tv( v.begin(), v.end() );
  /// remove the "dummy" vertices
  tv.erase( std::remove_if(tv.begin(), tv.end(), dummyVertex ), tv.end());
  return tv;
}

template<typename T>
std::vector<VertexView> makevertexview( const std::vector<T>& v ) {
  std::vector<VertexView> tv( v.begin(), v.end() );
  tv.erase( std::remove_if(tv.begin(), tv.end(), dummyVertex ), tv.end());
  return tv;
}


template<typename T>
std::vector<VertexView> makevertexview( typename T::const_iterator ibegin, typename T::const_iterator iend ) {
  std::vector<VertexView> tv( ibegin, iend );
  tv.erase( std::remove_if(tv.begin(), tv.end(), dummyVertex), tv.end());
  return tv;
}


#endif  // TRIGIDR4MONITORING_VERTEXVIEW_H 










