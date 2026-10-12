/* emacs: this is -*- c++ -*- */
/**
 **   @file    ParentView.h        
 **                   
 **            a TrackView that can contain children
 **
 **   @author  sutt
 **   @date    Sat 25 July 2026 19:21:14 BST
 **
 **   $Id: ParentView.h, v0.0   Sat 25 July 2026 19:21:14 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRIGIDR4MONITORING_PARENTVIEW_H
#define  TRIGIDR4MONITORING_PARENTVIEW_H

#include "ChildView.h"
#include "TrackView.h"

#include <vector>
#include <iostream>


/// a TrackView that can store child TrackViews - can be used to 
/// store the base TrackParticle for an electron, or the constituent 
/// particles for a Tau, or jet particles, or the lepton pairs for
/// Z bosons etc

class ParentView : public TrackView {

public:

  template<typename T>
  ParentView(const T* pobj) : TrackView( pobj ) { addChildren(); }
  
  ParentView( const ParentView& t ) = default;
  
  ParentView( const ParentView* ) = delete; // don't want implicitly creating a ParentView with an internal ParentView*
  ParentView(       ParentView* ) = delete;

  bool  isComposite() const override { return true; }

  bool  hasChildren() const override { return m_children.size()>0; }
  
  size_t childCount() const override { return m_children.size(); }

  const TrackView* child(size_t i) const override { return &m_children[i]; }

  std::vector<ChildView>::const_iterator begin() const { return m_children.begin(); } 
  std::vector<ChildView>::const_iterator   end() const { return m_children.end();   } 
  
  void claimChildren() {
    for ( ChildView& c : m_children ) c.setParent( this );
  }
  
private:

  /// for populating with the children ...
  
  void addChildren() { vtable()->addChildren( ptr(), m_children ); }

private:
  
  std::vector<ChildView>  m_children;

};


#endif  // TRIGIDR4MONITORING_PARENTVIEW_H 










