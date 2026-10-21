/* emacs: this is -*- c++ -*- */
/**
 **   @file    TruthView.h        
 **                   
 **            a TrackView that can store a z0, d0 and phi value 
 **            because the TruthParticles only have production 
 **            vertices and we don't want to be messing around
 **            ruining the code by passing the beamline around everywhere
 **            and we really can't rely on  
 **            static thread_local SG::ConstAccessor<float> z0Acc("z0");
 **            because z0Acc isAvailable(p) is usually false 
 **
 **   @author  sutt
 **   @date    Sat 25 July 2026 19:21:14 BST
 **
 **   $Id: TruthView.h, v0.0   Sat 25 July 2026 19:21:14 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRIGIDR4MONITORING_TRUTHVIEW_H
#define  TRIGIDR4MONITORING_TRUTHVIEW_H

#include "TrackView.h"


/// a TrackView that can store a z0, d0, and phi because a TruthParticle
/// doesn't have them

class TruthView : public TrackView {

public:

  template<typename T>
  TruthView(const T* pobj) : TrackView(pobj), m_z0(0), m_d0(0), m_phi(0) { }
  
  TruthView( const TruthView& t ) = default;
  
  TruthView( const TruthView* ) = delete; // don't want implicitly creating a TruthView with an internal TruthView*
  TruthView(       TruthView* ) = delete;

  virtual ~TruthView() = default;

  bool hasDecorations() const override { return true; }
  
  float  z0() const override { return m_z0;  }
  float  d0() const override { return m_d0;  }
  float phi() const override { return m_phi; }

  void  add_z0( float z0  ) { m_z0  = z0;  } 
  void  add_d0( float d0  ) { m_d0  = d0;  } 
  void add_phi( float phi ) { m_phi = phi; } 
  
private:

  float  m_z0;
  float  m_d0;
  float  m_phi;
 
};


#endif  // TRIGIDR4MONITORING_TRUTHVIEW_H 










