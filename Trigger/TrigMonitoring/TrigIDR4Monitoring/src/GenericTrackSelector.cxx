/**
 **   @file    GenericTrackSelector.cxx
 ** 
 **            Note that these all use Gaud::Property<ConfoguredValue<T>> 
 **            parameters which are a simple, yet effective wrapper around
 **            the values in the Gaudi::Property that knows whether it has
 **            been set with a value, or not so when performaing a comparison,
 **            if is not actually set the bool test returns a configurable,
 **            default value, either true, or false (false by default), so no  
 **            guard values, or tests on guard values are explicitly required
 **
 **   @author  sutt 
 **   @date    Fri 13 Mar 2026 23:01:14 GMT
 **
 **   $Id: GenericTrackSelector.cxx, v0.0   Fri 13 Mar 2026 23:01:14 GMT sutt $
 **
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#include "xAODEgamma/ElectronContainer.h"

#include "GenericTrackSelector.h"

#include <cmath>


IDTPM2::GenericTrackSelector::GenericTrackSelector( const std::string& name )
  : asg::AsgTool( name ) { }



StatusCode IDTPM2::GenericTrackSelector::initialize() {
  if ( m_initialized ) return StatusCode::SUCCESS;
  ATH_CHECK( asg::AsgTool::initialize() );  
  m_initialized = true;
  return StatusCode::SUCCESS;
}


  
bool IDTPM2::GenericTrackSelector::selectTrack( const TrackView& t ) const {

  /// Customised selections

  /// see the comment above - all these tests are only actually
  /// applied if the value has been set in the job option. If it
  /// is not set, then the comparison is not actually performed
  
  if ( t.pt() < m_minPt )                return false;  
  if ( t.pt() > m_maxPt )                return false;

  /// specifically for electrons, but by default, cut will be 0
  /// so we can apply it generally - should probably introduce
  /// explicit parent cut parameters to make things more flexible 

  if ( t.hasParent() ) {
    if ( t.parent()->ET() < m_minET     ) return false;
    if ( std::fabs(t.parent()->ET()/t.pt()) < m_minETovpt ) return false;
  }
  else { 
    if ( t.ET() < m_minET     ) return false;
    if ( std::fabs(t.ET()/t.pt()) < m_minETovpt ) return false;
  }
    
  if ( t.eta() > m_maxEta )              return false;
  if ( t.eta() < m_minEta )              return false; 
  if ( std::fabs(t.eta()) < m_minAbsEta )       return false; 

  if ( t.d0() < m_minD0 )                return false; 
  if ( t.d0() > m_maxD0 )                return false; 
  if ( t.z0() < m_minZ0 )                return false; 
  if ( t.z0() > m_maxZ0 )                return false; 

  if ( std::fabs(t.d0()) < m_minAbsD0 )         return false; 
  if ( std::fabs(t.d0()) > m_maxAbsD0 )         return false; 
  if ( std::fabs(t.z0()) < m_minAbsZ0 )         return false; 
  if ( std::fabs(t.z0()) > m_maxAbsZ0 )         return false; 

  if ( t.phi() < m_minPhi )              return false; 
  if ( t.phi() > m_maxPhi )              return false;  

  /// Abs phi ???? why would anyione want to select on this ?? 
  //  if ( std::fabs(t.phi()) < m_minAbsPhi )       return false; 
  //  if ( std::fabs(t.phi()) > m_maxAbsPhi )       return false;

  /// just to save one division ??
  ///  if ( isset(m_minQoPT) || isset(m_maxQoPT) || isset(m_minAbsQoPT) || isset(m_maxAbsQoPT) ) { 
  
  double qoverpt = t.qOverP()/std::sin(t.theta()); 

  if ( qoverpt < m_minQoPT )         return false; 
  if ( qoverpt > m_maxQoPT )         return false; 
  
  /// if we are taking the abs of qoverpt, that is the same as abs(1/pt)
  /// assuming we never have to deal with particles with charges other than +-1
  /// (or course, quarks don't count) 
  if ( std::fabs(qoverpt) < m_minAbsQoPT )  return false; 
  if ( std::fabs(qoverpt) > m_maxAbsQoPT )  return false;
  
  //}

  return true;
}

