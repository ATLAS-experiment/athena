/**
 **     @file    TagNProbe.cxx
 **
 **     @author  mark sutton
 **     @date    Sun Aug  2 17:59:15 CEST 2026
 **
 **     Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/


#include "TagNProbe.h"


IDTPM2::TagNProbe::TagNProbe( double massMin, double massMax ) :
  m_massMin(massMin),
  m_massMax(massMax)
{ }



bool IDTPM2::TagNProbe::select( const TrackView* t ) { 

    /// first check whether the masses have been generated before ...
    map_t::const_iterator itr = m_masses.find( t );
    
    if ( itr!=m_masses.end() ) return (itr->second).size()>0;
    
    std::vector<double> masses;

    masses.reserve( m_tags.size() );
    
    double z0 = getZ(t);
    
    for ( size_t i=0; i<m_tags.size() ; i++ ) {

      bool ok = false;
      
      if ( t->ptr()==m_tags[i]->ptr() ) continue;
      
      /// check compatibility of the track z and invariant mass ...
      double invmass = 0;
      
      if ( t->hasParent() && m_tags[i]->hasParent() ) invmass = mass( m_tags[i]->parent(), t->parent() );
      else invmass = mass( m_tags[i], t );                                           
      
      if ( invmass<m_massMin || invmass>m_massMax ) continue;

      /// ok, so mass is ok, now checke the z ...
      
      double z1 = getZ( m_tags[i] );

      if ( z0 && z1 ) { 
	/// do we want a programable parameter ? could just look at the dz I guess
	if ( std::fabs(z0-z1)<5 ) ok = true;
      }
      else ok = true;
      
      if ( ok ) masses.push_back( invmass );
      
    }

    /// don't need the mass - will need an empty vector if no masses in the range
    ///    if ( masses.size() ) m_masses.insert( map_t::value_type( t, masses ) );
    m_masses.insert( map_t::value_type( t, masses ) );

    return (masses.size()>0);
    
}

  
template<typename T1, typename T2>
double IDTPM2::TagNProbe::mass( const T1* t1, const T2* t2 ) const {
  TLorentzVector v1;
  v1.SetPtEtaPhiM( t1->pt()*0.001, t1->eta(), t1->phi(), t1->m()*0.001 );
  TLorentzVector v2;
  v2.SetPtEtaPhiM( t2->pt()*0.001, t2->eta(), t2->phi(), t2->m()*0.001 );
  return (v1+v2).M();    
}



/// but what if there are no z values ???
/// and what is the offline reconstruction has stupidly just
/// added tracks which are no were near each other in z ??
double IDTPM2::TagNProbe::getZ( const TrackView* t ) const {
  double z = 0;
  /// if we are taking the mean this way, we should probably
  /// check they are consistent ... 
  if ( t->hasChildren() && t->childCount()>0 ) { 
    for ( size_t i=0 ; i<t->childCount() ; i++ ) z += t->child(i)->z0();
    z /= t->childCount();
  }
  else z = t->z0();
  return z;
}



