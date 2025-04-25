/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TruthQualitySelectionTool.h"
#include "TrackParametersHelper.h"
#include "TruthUtils/HepMCHelpers.h"


IDTPM::TruthQualitySelectionTool::TruthQualitySelectionTool( const std::string& name )
  : asg::AsgTool( name ) { }

StatusCode IDTPM::TruthQualitySelectionTool::initialize()
{
  ATH_CHECK( m_truthTool.retrieve() );
  ATH_CHECK( m_trackTruthOriginTool.retrieve() );
  return StatusCode::SUCCESS;
}


StatusCode IDTPM::TruthQualitySelectionTool::selectTracks( TrackAnalysisCollections& trkAnaColls )
{
  std::vector< const xAOD::TruthParticle* > selected;
  for( const xAOD::TruthParticle* truth :
        trkAnaColls.truthPartVec( TrackAnalysisCollections::FS ) ) {
    if( accept( truth ) ) selected.push_back( truth );
  }

  ATH_MSG_DEBUG( "Size before selection: " <<
                 trkAnaColls.truthPartVec( TrackAnalysisCollections::FS ).size() <<
                 "\t Size after selection: " << selected.size() );

  /// updating FS collection
  ATH_CHECK( trkAnaColls.fillTruthPartVec( selected, TrackAnalysisCollections::FS ) );

  return StatusCode::SUCCESS;
}


bool IDTPM::TruthQualitySelectionTool::accept( const xAOD::TruthParticle* truth )
{
  /// Baseline selection, via AthTruthSelectionTool
  if( not m_truthTool->accept( truth ) )                            return false;

  /// Customised selections
  if (m_maxEta!=-9999.  and (eta(*truth)) > m_maxEta )              return false;
  if (m_minEta!=-9999.  and (eta(*truth)) < m_minEta )              return false;
  if (m_minPhi!=-9999.  and (phi(*truth)) < m_minPhi )              return false;
  if (m_maxPhi!=-9999.  and (phi(*truth)) > m_maxPhi )              return false;
  if (m_minD0!=-9999.   and (d0(*truth)) < m_minD0 )                return false;
  if (m_maxD0!=-9999.   and (d0(*truth)) > m_maxD0 )                return false;
  if (m_minZ0!=-9999.   and (z0(*truth)) < m_minZ0 )                return false;
  if (m_maxZ0!=-9999.   and (z0(*truth)) > m_maxZ0 )                return false;
  if (m_minQoPT!=-9999. and (qOverPT(*truth)) < m_minQoPT )         return false;
  if (m_maxQoPT!=-9999. and (qOverPT(*truth)) > m_maxQoPT )         return false;
  if (m_minAbsEta!=-9999.  and std::fabs(eta(*truth)) < m_minAbsEta )       return false;
  if (m_minAbsPhi!=-9999.  and std::fabs(phi(*truth)) < m_minAbsPhi )       return false;
  if (m_maxAbsPhi!=-9999.  and std::fabs(phi(*truth)) > m_maxAbsPhi )       return false;
  if (m_minAbsD0!=-9999.   and std::fabs(d0(*truth)) < m_minAbsD0 )         return false;
  if (m_maxAbsD0!=-9999.   and std::fabs(d0(*truth)) > m_maxAbsD0 )         return false;
  if (m_minAbsZ0!=-9999.   and std::fabs(z0(*truth)) < m_minAbsZ0 )         return false;
  if (m_maxAbsZ0!=-9999.   and std::fabs(z0(*truth)) > m_maxAbsZ0 )         return false;
  if (m_minAbsQoPT!=-9999. and std::fabs(qOverPT(*truth)) < m_minAbsQoPT )  return false;
  if (m_maxAbsQoPT!=-9999. and std::fabs(qOverPT(*truth)) > m_maxAbsQoPT )  return false;
  if (m_isHadron           and not isHadron(*truth) )                       return false;
  if (m_isPion             and not isPion(*truth) )                         return false;
  bool isFromBdecay = m_trackTruthOriginTool->isFrom( truth, MC::BQUARK );
  bool isFromCdecay = m_trackTruthOriginTool->isFrom( truth, MC::CQUARK );
  bool isFromHeavy = isFromBdecay or isFromCdecay;
  if( m_isFromB           and not isFromBdecay )              return false;
  if( m_isFromC           and not isFromCdecay )              return false;
  if( m_isFromHeavyFlav   and not isFromHeavy )               return false;
  if( m_isFromLightFlav   and isFromHeavy )                   return false;

  return true;
}
