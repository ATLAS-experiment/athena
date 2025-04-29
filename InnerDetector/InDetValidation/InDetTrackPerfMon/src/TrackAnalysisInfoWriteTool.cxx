/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackAnalysisInfoWriteTool.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// Athena includes
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"

/// Local includes
#include "TrackAnalysisInfoWriteTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackMatchingLookup.h"

/// STL includes
#include <sstream>


///---------------------------
///------- Constructor -------
///---------------------------
IDTPM::TrackAnalysisInfoWriteTool::TrackAnalysisInfoWriteTool(
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackAnalysisInfoWriteTool::initialize()
{
  ATH_MSG_DEBUG( "Initializing " << name() );

  ATH_CHECK( asg::AsgTool::initialize() );

  /// Retrieving TrkAnaDefSvc
  m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc" + m_anaTag.value());
  ATH_CHECK( m_trkAnaDefSvc.isValid() );

  return StatusCode::SUCCESS;
}


///------------------
///----- write ------
///------------------
StatusCode IDTPM::TrackAnalysisInfoWriteTool::write(
    SG::WriteHandle< xAOD::BaseContainer >& wh,
    TrackAnalysisCollections& trkAnaColls,
    const std::string& chain,
    unsigned int roiIdx,
    const std::string& roiStr ) const
{
  ATH_MSG_DEBUG( "Writing TrackAnalysisInfo to StoreGate" );

  if( not wh.key().ends_with( m_anaTag.value() ) ) {
    ATH_MSG_ERROR( "Invalid TrkAnaInfo container name: " << wh.key() );
    return StatusCode::FAILURE;
  }

  /// Accessors
  static const SG::Accessor< std::string >  chainAcc( "chain" );
  static const SG::Accessor< unsigned int > roiIdxAcc( "roiIdx" );
  static const SG::Accessor< std::string >  roiStrAcc( "roiStr" );
  static const SG::Accessor< VecEL_t<xAOD::TrackParticleContainer> >  testTracksAcc( "testTracks" );
  static const SG::Accessor< VecEL_t<xAOD::TruthParticleContainer> >  testTruthsAcc( "testTruths" );
  static const SG::Accessor< VecEL_t<xAOD::TrackParticleContainer> >  refTracksAcc( "refTracks" );
  static const SG::Accessor< VecEL_t<xAOD::TruthParticleContainer> >  refTruthsAcc( "refTruths" );
  static const SG::Accessor< VecF_t >  matchDistsAcc( "matchDists" );

  /// adding new element
  wh->push_back( std::make_unique< SG::AuxElement >() );

  /// Filling Trigger navigation info
  if( m_trkAnaDefSvc->doTrigNavigation() ) {
    chainAcc( *(wh->back()) ) = chain;
    roiIdxAcc( *(wh->back()) ) = roiIdx;
    roiStrAcc( *(wh->back()) ) = roiStr;
  }

  /// filling info vectors
  /// Track->Truth
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    MatchInfo_t< xAOD::TrackParticleContainer, xAOD::TruthParticleContainer > 
        matchInfo = getMatchInfo( 
            trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.testTrackContainer(),
            trkAnaColls.refTruthContainer(),
            &(trkAnaColls.matches()) );

    testTracksAcc( *(wh->back()) ) = std::get<0>( matchInfo );
    refTruthsAcc( *(wh->back()) ) = std::get<1>( matchInfo );
    matchDistsAcc( *(wh->back()) ) = std::get<2>( matchInfo );
  }
  /// Truth->Track
  else if( m_trkAnaDefSvc->isTestTruth() ) {
    MatchInfo_t< xAOD::TruthParticleContainer, xAOD::TrackParticleContainer > 
        matchInfo = getMatchInfo( 
            trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.testTruthContainer(),
            trkAnaColls.refTrackContainer(),
            &(trkAnaColls.matches()) );

    testTruthsAcc( *(wh->back()) ) = std::get<0>( matchInfo );
    refTracksAcc( *(wh->back()) ) = std::get<1>( matchInfo );
    matchDistsAcc( *(wh->back()) ) = std::get<2>( matchInfo );
  }
  /// Track->Track
  else {
    MatchInfo_t< xAOD::TrackParticleContainer, xAOD::TrackParticleContainer > 
        matchInfo = getMatchInfo( 
            trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
            trkAnaColls.testTrackContainer(),
            trkAnaColls.refTrackContainer(),
            &(trkAnaColls.matches()) );

    testTracksAcc( *(wh->back()) ) = std::get<0>( matchInfo );
    refTracksAcc( *(wh->back()) ) = std::get<1>( matchInfo );
    matchDistsAcc( *(wh->back()) ) = std::get<2>( matchInfo );
  }

  return StatusCode::SUCCESS;
}


///----------------------
///----- printInfo ------
///----------------------
std::string IDTPM::TrackAnalysisInfoWriteTool::printInfo(
    SG::WriteHandle< xAOD::BaseContainer >& wh ) const
{
  if( not wh.isValid() ) {
    ATH_MSG_ERROR( "Invalid collection" ); return "";
  }

  /// Const accessors
  static const SG::ConstAccessor< std::string >  chainCAcc( "chain" );
  static const SG::ConstAccessor< unsigned int > roiIdxCAcc( "roiIdx" );
  static const SG::ConstAccessor< std::string >  roiStrCAcc( "roiStr" );
  static const SG::ConstAccessor< VecEL_t<xAOD::TrackParticleContainer> >  testTracksCAcc( "testTracks" );
  static const SG::ConstAccessor< VecEL_t<xAOD::TruthParticleContainer> >  testTruthsCAcc( "testTruths" );
  static const SG::ConstAccessor< VecEL_t<xAOD::TrackParticleContainer> >  refTracksCAcc( "refTracks" );
  static const SG::ConstAccessor< VecEL_t<xAOD::TruthParticleContainer> >  refTruthsCAcc( "refTruths" );
  static const SG::ConstAccessor< VecF_t >  matchDistsCAcc( "matchDists" );

  std::stringstream ss;
  ss << wh.key() << " : " << m_trkAnaDefSvc->testType()
     << " --> " << m_trkAnaDefSvc->referenceType() << " ( dist ) :\n";

  for( size_t icr=0; icr<wh->size(); icr++ ) {
    std::string chain = chainCAcc.isAvailable( *(wh->at(icr)) ) ?
                        chainCAcc( *(wh->at(icr)) ) : "Offline";
    std::string roiIdx = roiIdxCAcc.isAvailable( *(wh->at(icr)) ) ?
                         std::to_string( roiIdxCAcc( *(wh->at(icr)) ) ) : "N/A";
    std::string roiStr = roiStrCAcc.isAvailable( *(wh->at(icr)) ) ?
                         roiStrCAcc( *(wh->at(icr)) ) : "N/A";

    ss << "\t\tChain : " << chain << "\n";
    ss << "\t\tRoI ( " << roiIdx << " ) : " << roiStr << "\n";
    ss << "\t\tMatch Info (pT) : " << "\n";

    /// Track->Truth
    if( m_trkAnaDefSvc->isReferenceTruth() ) {
      ss << printMatchInfo< xAOD::TrackParticleContainer, xAOD::TruthParticleContainer >(
              testTracksCAcc( *(wh->at(icr)) ),
              refTruthsCAcc( *(wh->at(icr)) ),
              matchDistsCAcc( *(wh->at(icr)) ) );
    }
    /// Truth->Track
    else if( m_trkAnaDefSvc->isTestTruth() ) {
      ss << printMatchInfo< xAOD::TruthParticleContainer, xAOD::TrackParticleContainer >(
              testTruthsCAcc( *(wh->at(icr)) ), 
              refTracksCAcc( *(wh->at(icr)) ),
              matchDistsCAcc( *(wh->at(icr)) ) );
    }
    /// Track->Track
    else {
      ss << printMatchInfo< xAOD::TrackParticleContainer, xAOD::TrackParticleContainer >(
              testTracksCAcc( *(wh->at(icr)) ),
              refTracksCAcc( *(wh->at(icr)) ),
              matchDistsCAcc( *(wh->at(icr)) ) );
    }

  } // close loop over collection

  return ss.str();
}
