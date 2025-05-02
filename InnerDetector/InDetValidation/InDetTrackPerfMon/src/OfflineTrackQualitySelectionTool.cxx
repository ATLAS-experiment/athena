/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "OfflineTrackQualitySelectionTool.h"
#include "TrackAnalysisCollections.h"
#include "TrackParametersHelper.h"
#include "OfflineObjectDecorHelper.h"

namespace IDTPM {

OfflineTrackQualitySelectionTool::OfflineTrackQualitySelectionTool( const std::string& name )
  : asg::AsgTool( name ) { }

StatusCode OfflineTrackQualitySelectionTool::initialize()
{
  ATH_CHECK( asg::AsgTool::initialize() );  
  ATH_CHECK( m_offlineTool.retrieve() );
  return StatusCode::SUCCESS;
}

StatusCode OfflineTrackQualitySelectionTool::selectTracks(
    TrackAnalysisCollections& trkAnaColls )
{
  std::vector< const xAOD::TrackParticle* > selected;
  for( const xAOD::TrackParticle* track :
       trkAnaColls.offlTrackVec( TrackAnalysisCollections::FS ) ) {
    if( accept( track ) ) selected.push_back( track );
  }

  ATH_MSG_DEBUG( "Size before selection: " <<
                 trkAnaColls.offlTrackVec( TrackAnalysisCollections::FS ).size() <<
                 "\t Size after selection: " << selected.size() );

  /// updating FS collection
  ATH_CHECK( trkAnaColls.fillOfflTrackVec( selected, TrackAnalysisCollections::FS ) );

  return StatusCode::SUCCESS;
}

bool OfflineTrackQualitySelectionTool::accept( const xAOD::TrackParticle* track )
{
  /// Baseline selection, via InDetTrackSelectionTool
  if ( not m_offlineTool->accept( track ) )           return false; // TODO vertex needs to be provided here

  /// Customised selections
  if (m_maxPt!=-9999.   and (pT(*track)) > m_maxPt )                return false;  
  if (m_maxEta!=-9999.  and (eta(*track)) > m_maxEta )              return false;
  if (m_minEta!=-9999.  and (eta(*track)) < m_minEta )              return false; 
  if (m_minPhi!=-9999.  and (phi(*track)) < m_minPhi )              return false; 
  if (m_maxPhi!=-9999.  and (phi(*track)) > m_maxPhi )              return false;  
  if (m_minD0!=-9999.   and (d0(*track)) < m_minD0 )                return false; 
  if (m_minZ0!=-9999.   and (z0(*track)) < m_minZ0 )                return false; 
  if (m_minQoPT!=-9999. and (qOverPT(*track)) < m_minQoPT )         return false; 
  if (m_maxQoPT!=-9999. and (qOverPT(*track)) > m_maxQoPT )         return false; 
  if (m_minAbsEta!=-9999.  and std::fabs(eta(*track)) < m_minAbsEta )       return false; 
  if (m_minAbsPhi!=-9999.  and std::fabs(phi(*track)) < m_minAbsPhi )       return false; 
  if (m_maxAbsPhi!=-9999.  and std::fabs(phi(*track)) > m_maxAbsPhi )       return false;  
  if (m_minAbsD0!=-9999.   and std::fabs(d0(*track)) < m_minAbsD0 )         return false; 
  if (m_maxAbsD0!=-9999.   and std::fabs(d0(*track)) > m_maxAbsD0 )         return false; 
  if (m_minAbsZ0!=-9999.   and std::fabs(z0(*track)) < m_minAbsZ0 )         return false; 
  if (m_maxAbsZ0!=-9999.   and std::fabs(z0(*track)) > m_maxAbsZ0 )         return false; 
  if (m_minAbsQoPT!=-9999. and std::fabs(qOverPT(*track)) < m_minAbsQoPT )  return false; 
  if (m_maxAbsQoPT!=-9999. and std::fabs(qOverPT(*track)) > m_maxAbsQoPT )  return false;
  if ( !m_etaBins.empty() ) {
    if (!m_minHitsVec.empty() and !nHitsSelVec(*track, m_minHitsVec, m_etaBins))  return false;
    if (!m_minPtVec.empty() and !minPtSelVec(*track, m_minPtVec, m_etaBins))  return false;
    if (!m_maxD0Vec.empty() and !maxD0SelVec(*track, m_maxD0Vec, m_etaBins))  return false;
    if (!m_maxZ0Vec.empty() and !maxZ0SelVec(*track, m_maxZ0Vec, m_etaBins))  return false;
  }

  return true;
}

}  // namespace IDTPM
