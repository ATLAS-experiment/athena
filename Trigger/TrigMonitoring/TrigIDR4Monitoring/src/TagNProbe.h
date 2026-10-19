/// emacs: this is -*- c++ -*-
/**
 **     @file    TagNProbe.h
 **
 **              this is a much simpler class than previously, since it actually 
 **              better describes what is needed. The important thing for a 
 **              TagNProbe analysis, is that the tnp selection is an *object*
 **              selection and *not*: an Roi selection, so we are given all the
 **              tag objects, and then for any give probe object, we ask if  
 **              it gives us a tag and probe candidate - that is it. Then this
 **              must be asked for every object being selected at the *object*
 **              level, not the trigger level, and not the roi level 
 **              incidentally, if we assume that the objects provided have the
 **              correct masses ( t.m() ) then we don't even need to store the
 **              required masses
 **
 **
 **     @author  mark sutton
 **     @date    Sun Aug  2 12:55:17 CEST 2026
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef IDTPM2_TAGNPROBE_H
#define IDTPM2_TAGNPROBE_H

#include <vector> 

#include "TrackView.h" 

#include "TLorentzVector.h"



namespace IDTPM2 { 

class TagNProbe {

protected:

  using map_t = std::unordered_map<const TrackView*, std::vector<double> >;

#if 0
  /// not used yet, just stick with the raw masses
  /// unless we want post-run selection on the
  /// the tag candidate.
  struct Resonance {
    const TrackView* m_tag;
    double           m_mass; 
  };
#endif
  
public:

  TagNProbe( double massMin, double massMax );
  
  virtual ~TagNProbe() { }
  
  void append_tags( const std::vector<const TrackView*>& v ) {
    append_tags( v.begin(), v.end() );
  }

  template<typename Iterator>
  void append_tags( Iterator first, Iterator last ) {
    m_tags.insert( m_tags.end(), first, last );
  }

  template<typename Iterator, typename Predicate>
  void append_tags( Iterator first, Iterator last, const Predicate& predicate ) {
    for ( ; first!=last ; first++  ) if ( predicate(*first) ) m_tags.push_back(*first);
  }

  
  bool select( const TrackView* t );

  const std::vector<double>& masses( const TrackView* t ) const {
    map_t::const_iterator itr = m_masses.find( t );
    if ( itr!=m_masses.end() ) return itr->second;
    throw std::runtime_error("object not found");
  }
  
  size_t size() const { return m_tags.size(); } 
  
protected:

  /// doesn't need to be a template ... 
  template<typename T1, typename T2>
  double mass( const T1* t1, const T2* t2 ) const;

  /// but what if there are no z values ???
  /// and what is the offline reconstruction has stupidly just
  /// added tracks which are no were near each other in z ??
  double getZ( const TrackView* t ) const;
  

private:

  double m_massMin;
  double m_massMax;
  
  std::vector<const TrackView*> m_tags;

  map_t m_masses;

};

}


#endif ///  IDTPM2_TAGNPROBE_H


