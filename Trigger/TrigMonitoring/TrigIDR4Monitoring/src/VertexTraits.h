// emacs: this is -*- c++ -*-
/**
 **   @file    VertexTraits.h
 **
 **            so far we only want to use the common
 **            x(), y() and z() mothods, so we don't
 **            concreate instances at the moment
 **                   
 **   @author  sutt
 **   @date    Wed 11 Mar 2026 17:54:21 GMT
 **
 **   $Id: VertexTraitsA.h, v0.0   Wed 11 Mar 2026 17:54:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  VERTEXTRAITS_H
#define  VERTEXTRAITS_H

#include <cstdint>
#include <cmath>
#include <stdexcept>


template<typename T>
struct VertexTraitsBase {
  
  static float  x(const T& v) { return v.x(); }
  static float  y(const T& v) { return v.y(); }
  static float  z(const T& v) { return v.z(); }
  
  static float  dx(const T& ) { return 0; }
  static float  dy(const T& ) { return 0; }
  static float  dz(const T& ) { return 0; }
  
  static size_t    nTrackParticles(const T&) { return 0;  }

  static float       chiSquared(const T& ) { return 0;  }
  static float        numberDoF(const T& ) { return 0;  }

  static float             time(const T& ) { return 0; } 

  static int         vertexType(const T& ) { return 0; }

};

template<typename T>
struct VertexTraits : public VertexTraitsBase<T> { };

template<typename T>
struct VertexTraits<const T> : VertexTraits<T> { };



#endif  // VERTEXTRAITS_H 







