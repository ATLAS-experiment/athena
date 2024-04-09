/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackRoiSelectionTool.cxx
 * @author marco aparo
 **/

/// Athena include(s)
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

/// Gaudi includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Local include(s)
#include "TrackRoiSelectionTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackParmetersHelper.h"

/// STD includes
#include <cmath> // std::fabs


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TrackRoiSelectionTool::TrackRoiSelectionTool(
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackRoiSelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_INFO( "Initializing " << name() );

  ATH_CHECK( m_triggerTrkParticleName.initialize( 
      not m_triggerTrkParticleName.key().empty() ) );

  ATH_CHECK( m_trigDecTool.retrieve() );

  return StatusCode::SUCCESS;
}


///-----------------------
///------- accept --------
///-----------------------
template< class T >
bool IDTPM::TrackRoiSelectionTool::accept(
    const T& t, const TrigRoiDescriptor* r ) const {

  if( r==0 ) { 
    ATH_MSG_ERROR( "Called with null RoiDescriptor" );
    return true;
  }

  if( r->composite() ) {       

    for( unsigned i=r->size() ; i-- ; )
      if( accept( t, (const TrigRoiDescriptor*)r->at(i) ) )
        return true;

  } else { 

    if( r->isFullscan() ) return true;

    /// NB: This isn't actually correct - the tracks can bend out of the 
    ///     RoI even if the perigee phi is within the Roi
    bool contained_phi = ( r->phiMinus() < r->phiPlus() ) ?
                         ( phi(t) > r->phiMinus() && phi(t) < r->phiPlus() ) :
                         ( phi(t) > r->phiMinus() || phi(t) < r->phiPlus() ); 
                            
    bool contained_zed = ( z0(t) > r->zedMinus() && z0(t) < r->zedPlus() ); 
    
    /// NB: This is *completely* wrong, tracks could be completely contained 
    ///     within an RoI but still fail this condition
    bool contained_eta = ( eta(t) < r->etaPlus() && eta(t) > r->etaMinus() );
                                                         
    ///  calculation of approximate z position of the 
    ///  track at radius r and test if track within that z position at radius r 
    exitPoint_t exit = getExitPoint( z0(t), eta(t) );

    /// full check to determine whether a track is  
    /// fully contained in the Roi or not (when used in conjunction 
    /// with contained_zed above)
    exitPoint_t exitPlus  = getExitPoint( r->zedPlus(),  r->etaPlus() );
    exitPoint_t exitMinus = getExitPoint( r->zedMinus(), r->etaMinus() );

    float cross0 = exit.z * exitMinus.r - exit.r * exitMinus.z;
    float cross1 = exit.z * exitPlus.r  - exit.r * exitPlus.z; 

    contained_eta = ( cross0>0 && cross1<0 ) ? true : false;

    /// now check phi taking account of the track transverse curvature
    float newphi = getOuterPhi( pTsig(t), phi(t), exit.r );
  
    if( newphi < -999. ) return false;
  
    if( r->phiMinus() < r->phiPlus() ) contained_phi &= ( newphi > r->phiMinus() && newphi < r->phiPlus() );
    else                               contained_phi &= ( newphi > r->phiMinus() || newphi < r->phiPlus() );

    if( contained_eta && contained_phi && contained_zed ) return true;
  }

  return false;
}

/// accept method for offline tracks
template bool IDTPM::TrackRoiSelectionTool::accept(
    const xAOD::TrackParticle& t, const TrigRoiDescriptor* r ) const;

/// accept method for truth particles
template bool IDTPM::TrackRoiSelectionTool::accept(
    const xAOD::TruthParticle& t, const TrigRoiDescriptor* r ) const;


///---------------------------
///------- getOuterPhi -------
///---------------------------
float IDTPM::TrackRoiSelectionTool::getOuterPhi(
    float pt, float phi, float r ) const {

  /// calculate the (very approximate) phi position 
  /// for a track at a radius r

  /// track (signed) radius of curvature
  float mqR = 10 * pt / ( 2.99792458 * 2 ); // 2.998=speed of light, 2=Atlas B field   
  float ratio = 0.5 * r / mqR;    
  float newphi = phi;

  /// make sure it escapes the radius in question
  if( std::fabs( ratio ) > 1 ) return -9999.;

  /// calculate new position
  newphi -= std::asin( ratio );  
                          
  /// wrap to -pi to pi
  while( newphi < -M_PI ) newphi += 2 * M_PI;
  while( newphi > M_PI  ) newphi -= 2 * M_PI;

  return newphi;
}


///----------------------------
///------- getExitPoint -------
///----------------------------
exitPoint_t IDTPM::TrackRoiSelectionTool::getExitPoint( 
    float tz0, float teta ) const {

  exitPoint_t exitPoint;
  float rexit(0.), zexit(0.);
  const float maxRadius = 1000.;
  const float maxZed    = 2700.;

  if      ( teta < 0 ) zexit = -maxZed;
  else if ( teta > 0 ) zexit =  maxZed;
  else { 
    exitPoint.z = tz0;
    exitPoint.r = maxRadius; 
    exitPoint.tantheta = 1e16; // don't really want to use nan
    return exitPoint;
  }
  
  float tantheta = std::tan( 2 * std::atan( std::exp( -teta ) ) );
  
  rexit = ( zexit - tz0 ) * tantheta;

  /// leaves through the barrel side or front face?
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
  exitPoint.tantheta = tantheta;
  return exitPoint;
}


///-------------------------
///------- getTracks -------
///-------------------------
template< class T >
std::vector< const T* > IDTPM::TrackRoiSelectionTool::getTracks(
    const std::vector< const T* >& tvec,
    const TrigRoiDescriptor* r ) const {

  std::vector< const T* > selectedTracks;

  for( size_t it=0 ; it<tvec.size() ; it++ ) {
    const T* thisTrack = tvec.at(it);
    if( accept<T>( *thisTrack, r ) ) {
      selectedTracks.push_back( thisTrack );
    }
  }

  return selectedTracks;
}

/// getTracks method for offline tracks
template std::vector< const xAOD::TrackParticle* >
IDTPM::TrackRoiSelectionTool::getTracks< xAOD::TrackParticle >(
    const std::vector< const xAOD::TrackParticle* >& tvec,
    const TrigRoiDescriptor* r ) const;

/// getTracks method for truth particles
template std::vector< const xAOD::TruthParticle* >
IDTPM::TrackRoiSelectionTool::getTracks< xAOD::TruthParticle >(
    const std::vector< const xAOD::TruthParticle* >& tvec,
    const TrigRoiDescriptor* r ) const;


///-----------------------------
///------- getTrigTracks -------
///-----------------------------
std::vector< const xAOD::TrackParticle* >
IDTPM::TrackRoiSelectionTool::getTrigTracks( 
    const SG::ReadHandleKey< xAOD::TrackParticleContainer >& handleKey,
    const ElementLink< TrigRoiDescriptorCollection >& roiLink ) const {

  SG::ReadHandle<xAOD::TrackParticleContainer> handle( handleKey );

  std::vector< const xAOD::TrackParticle* > selectedTrigTracks;
  
  std::pair< xAOD::TrackParticleContainer::const_iterator,
             xAOD::TrackParticleContainer::const_iterator > selTrigTrkItrPair =
                 m_trigDecTool->associateToEventView( handle, roiLink );

  xAOD::TrackParticleContainer::const_iterator trigTrkiItr;
  for( trigTrkiItr = selTrigTrkItrPair.first ;
       trigTrkiItr != selTrigTrkItrPair.second ;
       trigTrkiItr++ ) {
    selectedTrigTracks.push_back( *trigTrkiItr );
  }  

  return selectedTrigTracks;
}


///-------------------------
///--- selectTracksInRoI ---
///-------------------------
StatusCode IDTPM::TrackRoiSelectionTool::selectTracksInRoI(
    TrackAnalysisCollections& trkAnaColls,
    const ElementLink< TrigRoiDescriptorCollection >& roiLink ) {

  ATH_MSG_DEBUG( "Selecting tracks in RoI" );

  /// retrieving TrkAnaDefSvc
  ITrackAnalysisDefinitionSvc* trkAnaDefSvc( nullptr );
  ISvcLocator* svcLoc = Gaudi::svcLocator();
  ATH_CHECK( svcLoc->service( "TrkAnaDefSvc"+trkAnaColls.anaTag(), trkAnaDefSvc ) );

  const TrigRoiDescriptor* const* roi = roiLink.cptr();

  /// Trigger tracks RoI selection
  ATH_CHECK( trkAnaColls.fillTestTrackVec(
      getTrigTracks( m_triggerTrkParticleName, roiLink ),
      TrackAnalysisCollections::InRoI ) );

  /// Offline tracks RoI selection
  if( trkAnaDefSvc->useOffline() ) {
    ATH_CHECK( trkAnaColls.fillOfflTrackVec(
        getTracks(
            trkAnaColls.offlTrackVec( TrackAnalysisCollections::FS ),
            *roi ),
        TrackAnalysisCollections::InRoI ) );
  }

  /// Truth particles RoI selection 
  if( trkAnaDefSvc->useTruth() ) {
    ATH_CHECK( trkAnaColls.fillTruthTrackVec(
        getTracks(
            trkAnaColls.truthTrackVec( TrackAnalysisCollections::FS ),
            *roi ),
        TrackAnalysisCollections::InRoI ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Tracks after RoI selection: " << 
      trkAnaColls.printInfo( TrackAnalysisCollections::InRoI ) );
 
  return StatusCode::SUCCESS;
}
