// emacs: this is -*- c++ -*-
/**
 **   @file    xAODVertexTraits.h        
 **
 **            annoyingly the *specefic* vertex class is
 **            called "Vertex" - how unuser friendly
 **
 **   @author  sutt
 **   @date    Thursday 12 Mar 2026 12:10:21 GMT
 **
 **   $Id: xAODTraits.h, v0.0   Thu 12 Mar 2026 12:10:21 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#ifndef  XAODVERTEXTRAITS_H
#define  XAODVERTEXTRAITS_H

#include "xAODTracking/VertexContainer.h"

#include "VertexTraits.h"


template<>
struct VertexTraits<xAOD::Vertex> : public VertexTraitsBase<xAOD::Vertex> {
  
public:
  
  static float   x(const xAOD::Vertex& v) { return v.x();  }
  static float   y(const xAOD::Vertex& v) { return v.y();  }
  static float   z(const xAOD::Vertex& v) { return v.z();  }

  static float  dx(const xAOD::Vertex& v) { return v.covariancePosition()(Trk::x,Trk::x);  }
  static float  dy(const xAOD::Vertex& v) { return v.covariancePosition()(Trk::y,Trk::y);  }
  static float  dz(const xAOD::Vertex& v) { return v.covariancePosition()(Trk::z,Trk::z);  }

  static size_t    nTrackParticles(const xAOD::Vertex& v) { return v.nTrackParticles();  }

  static float          chiSquared(const xAOD::Vertex& v) { return v.chiSquared(); }
  static float           numberDoF(const xAOD::Vertex& v) { return v.numberDoF();  }

  ///  for another day, lest we forget ...
  ///  static float           time(const xAOD::Vertex& v) { return v.time();  }

  static int            vertexType(const xAOD::Vertex& v) { return v.vertexType();  }

};

#endif // XAODVERTEXTRAITS_H
