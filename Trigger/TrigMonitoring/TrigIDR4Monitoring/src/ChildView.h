/* emacs: this is -*- c++ -*- */
/**
 **   @file    ChildView.h        
 **                   
 **            a TrackView that has an explicit parent
 **
 **   @author  sutt
 **   @date    Sat 25 July 2026 19:21:14 BST
 **
 **   $Id: ChildView.h, v0.0   Sat 25 July 2026 19:21:14 BST sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/


#ifndef  TRIGIDR4MONITORING_CHILDVIEW_H
#define  TRIGIDR4MONITORING_CHILDVIEW_H

#include "TrackView.h"


/// a TrackView that can have a parent to store the Electron, or Tau etc
/// Z bosons etc

class ChildView : public TrackView {

public:

  template<typename T>
  ChildView( const T* pobj ) : TrackView( pobj ), m_parent(0) { }
  
  ChildView( const ChildView& t ) = default;
  ChildView( const TrackView& t ) : TrackView(t), m_parent(0) { }
  
  ChildView( const ChildView* ) = delete; // don't want implicitly creating a ChildView with an internal ChildView*
  ChildView(       ChildView* ) = delete;

  virtual ~ChildView() = default;

  bool hasParent() const override { return m_parent; }
  
  const TrackView* parent() const override { return m_parent; }

  /// be careful with this - can ONLY be used once the parent
  /// container is fixed and won't / can't ever be moved
  
  void setParent( const TrackView* p ) { m_parent = p; } 
  
private:

  /// pointer to the parent - technically, should be a
  /// ParentView, but keeping everything as a TrackView*
  /// simplifies the interface - isn't polymorphism great !
  const TrackView* m_parent;

};



#endif  // TRIGIDR4MONITORING_CHILDVIEW_H 










