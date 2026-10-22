/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackRoiSelector.cxx
 * @author marco aparo
 **/

/// Athena include(s)
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// Local include(s)
#include "TrackRoiSelector.h"
#include "TrackView.h"

/// STD includes
#include <cmath> // std::fabs
#include <algorithm> // for std::find

#include "TrigInDetAnalysis/TIDARoiDescriptor.h"

///-----------------------
///------- accept --------
///-----------------------
template<class T>
bool IDTPM2::TrackRoiSelector::accept( const T& t, const TrigRoiDescriptor* r ) const {

  if ( r==0 ) return true; /// should really throw an exception 

  if ( t.pt()==0 ) return false; // throw runtime_error(shouldn't ever have a track with pt = 0" );
    
  if( r->composite() ) {       

    for ( unsigned i=r->size() ; i-- ; ) if ( accept( t, (const TrigRoiDescriptor*)r->at(i) ) ) return true;

  }
  else { 

    if( r->isFullscan() ) return true;

    //    std::cout << "r->
    
    /// NB: This isn't actually correct - the tracks can bend out of the 
    ///     RoI even if the perigee phi is within the Roi
    ///     so we check the outer phi limit later ...
    bool contained_phi = ( r->phiMinus()<r->phiPlus() ) ?
                         ( t.phi()>r->phiMinus() && t.phi()<r->phiPlus() ) :
                         ( t.phi()>r->phiMinus() || t.phi()<r->phiPlus() ); 

    /// must be contained along the beamline ... 
    bool contained_zed = ( t.z0()>r->zedMinus() && t.z0()<r->zedPlus() ); 
    
    ///     this is actually wrong, but hey ho. It is 
    ///     even needed ?
    bool contained_eta = ( t.eta()<r->etaPlus() && t.eta()>r->etaMinus() );
                                                         
    ///  calculation of approximate z position of the 
    ///  track at radius r && test if track within that z position at radius r 

#if 0
    /// the TIDARoiDescriptor ALREADY has the exit points of the Roi
    /// stored, we certainly don't need to recalculate them for EVERY track
    /// we want to check, need to port this functionality over to the real
    //  RoiDescriptor so that computes and stores it's exit points as standard

    exit_point ex;
    exit_point explus;
    exit_point exminus;

    TIDARoiDescriptor::exitpoint( t.z0(), t.eta(), ex.z, ex.r );

    TIDARoiDescriptor::exitpoint( r->zedPlus(),   r->etaPlus(),   explus.z,  explus.r );
    TIDARoiDescriptor::exitpoint( r->zedMinus(),  r->etaMinus(), exminus.z, exminus.r );

#else
    
    /// full check to determine whether a track is
    /// fully contained in the Roi (when used in conjunction 
    /// with contained_zed above)
    /// sadly this will recalculate the Roi limits for *every* track, which is
    /// why the earlier version took this stuff from the Roi itself - will need
    /// to go back to that at some point, 
    exit_point ex      = getExitPoint( t.z0(),  t.eta() );
    exit_point explus  = getExitPoint( r->zedPlus(),  r->etaPlus() );
    exit_point exminus = getExitPoint( r->zedMinus(), r->etaMinus() );
#endif

    float cross0 = ex.z*exminus.r - ex.r*exminus.z;
    float cross1 = ex.z*explus.r  - ex.r*explus.z; 

    contained_eta = ( cross0>0 && cross1<0 ) ? true : false;

    /// now check phi taking account of the track transverse curvature
    /// NB: need to check the pt is properly signed
    std::optional<double> newphi = getOuterPhi( t.pt(), t.phi(), ex.r );

    if( !newphi ) return false;
  
    if( r->phiMinus() < r->phiPlus() ) contained_phi &= ( newphi > r->phiMinus() && newphi < r->phiPlus() );
    else                               contained_phi &= ( newphi > r->phiMinus() || newphi < r->phiPlus() );

    if( contained_eta && contained_phi && contained_zed ) return true;
  }

  return false;
}



/// accept method for truth particles
template bool IDTPM2::TrackRoiSelector::accept( const TrackView& t, const TrigRoiDescriptor* r ) const;


///---------------------------
///------- getOuterPhi -------
///---------------------------
std::optional<double> IDTPM2::TrackRoiSelector::getOuterPhi( float pt, float phi, float r ) const {

  /// calculate the (very approximate) phi position 
  /// for a track at a radius r

  
  /// track (signed) radius of curvature (if pt is signed) 
  double mqR    = 10 * pt / ( 2.99792458 * 2 ); // 2.998=speed of light, 2=Atlas B field   
  double ratio  = 0.5 * r / mqR;    
  double newphi = phi;
  
  /// make sure it escapes the radius in question
  if ( std::fabs(ratio)>1 ) return std::nullopt;
  
  /// calculate new position
  newphi -= std::asin( ratio );  
                          
  /// wrap to -pi to pi - is this correct when using floats ? NB: float(M_PI)>M_PI  ?
  /// so use doubles for all the calculations
  while( newphi < -M_PI ) newphi += 2 * M_PI;
  while( newphi > M_PI  ) newphi -= 2 * M_PI;

  return newphi;
}


/// Actually, this class should really be implemented as inheriting from an ITrackSelector
/// 

#if 1
///----------------------------
///------- getExitPoint -------
///----------------------------
IDTPM2::TrackRoiSelector::exit_point IDTPM2::TrackRoiSelector::getExitPoint( float tz0, float teta ) const {

  exit_point exitPoint;

  double  rexit = 0;
  double  zexit = 0;
  const double maxRadius = 1000.;
  const double maxZed    = 2700.;

  if      ( teta < 0 ) zexit = -maxZed;
  else if ( teta > 0 ) zexit =  maxZed;
  else { 
    exitPoint.z = tz0;
    exitPoint.r = maxRadius; 
    return exitPoint;
  }
  
  double tantheta = std::tan( 2 * std::atan( std::exp( -teta ) ) );
  
  rexit = ( zexit - tz0 ) * tantheta; /// not quite right, because it might be passing out through the rear endcap 

  /// leaves through the barrel side || front face?
  if ( std::fabs(rexit) > maxRadius ) {
    /// through the barrel edge
    /// actually need to calculate the z exit coordinate                                                                              
    /// for proper containment rather than spurious
    /// "eta containment" 

    zexit = maxRadius / tantheta + tz0;
    rexit = maxRadius;
  }

  exitPoint.z = zexit;
  exitPoint.r = rexit;
  
  return exitPoint;
}

#endif
