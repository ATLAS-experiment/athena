/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackPerfMon/TruthQualitySelectionTool.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"
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



const xAOD::TruthParticle* IDTPM::TruthQualitySelectionTool::getParent(const xAOD::TruthParticle* truth, int flav) const {
  return getParentRec( truth, flav, 0 );
}

const xAOD::TruthParticle* IDTPM::TruthQualitySelectionTool::getParentRec(const xAOD::TruthParticle* truth, int flav, int depth) const {

  if ( truth == nullptr ) return nullptr;

  if ( depth > 30 ) return nullptr;

  if( flav != MC::BQUARK && flav != MC::CQUARK && flav != MC::TAU ) return nullptr;

  if( flav == MC::BQUARK && truth->isBottomHadron() ) return truth;

  if( flav == MC::CQUARK && truth->isCharmHadron() ) return truth;

  if( flav == MC::TAU && MC::isTau(truth) ) return truth;


  for(unsigned int p=0; p<truth->nParents(); p++) {
    const xAOD::TruthParticle* parent = truth->parent(p);
    if(parent == truth ) continue ; // avoid infinite recursion
    if( getParentRec(parent, flav, depth+1)!= nullptr ) return parent;
  }

  return nullptr;
}



bool IDTPM::TruthQualitySelectionTool::accept( const xAOD::TruthParticle* truth )
{
  /// Baseline selection, via AthTruthSelectionTool
  if( ! m_truthTool->accept( truth ) )                            return false;

  /// Customised selections
  if (m_maxEta!=-9999.  && (eta(*truth)) > m_maxEta )              return false;
  if (m_minEta!=-9999.  && (eta(*truth)) < m_minEta )              return false;
  if (m_minPhi!=-9999.  && (phi(*truth)) < m_minPhi )              return false;
  if (m_maxPhi!=-9999.  && (phi(*truth)) > m_maxPhi )              return false;
  if (m_minD0!=-9999.   && (d0(*truth)) < m_minD0 )                return false;
  if (m_maxD0!=-9999.   && (d0(*truth)) > m_maxD0 )                return false;
  if (m_minZ0!=-9999.   && (z0(*truth)) < m_minZ0 )                return false;
  if (m_maxZ0!=-9999.   && (z0(*truth)) > m_maxZ0 )                return false;
  if (m_minQoPT!=-9999. && (qOverPT(*truth)) < m_minQoPT )         return false;
  if (m_maxQoPT!=-9999. && (qOverPT(*truth)) > m_maxQoPT )         return false;
  if (m_minAbsEta!=-9999.  && std::fabs(eta(*truth)) < m_minAbsEta )       return false;
  if (m_minAbsPhi!=-9999.  && std::fabs(phi(*truth)) < m_minAbsPhi )       return false;
  if (m_maxAbsPhi!=-9999.  && std::fabs(phi(*truth)) > m_maxAbsPhi )       return false;
  if (m_minAbsD0!=-9999.   && std::fabs(d0(*truth)) < m_minAbsD0 )         return false;
  if (m_maxAbsD0!=-9999.   && std::fabs(d0(*truth)) > m_maxAbsD0 )         return false;
  if (m_minAbsZ0!=-9999.   && std::fabs(z0(*truth)) < m_minAbsZ0 )         return false;
  if (m_maxAbsZ0!=-9999.   && std::fabs(z0(*truth)) > m_maxAbsZ0 )         return false;
  if (m_minAbsQoPT!=-9999. && std::fabs(qOverPT(*truth)) < m_minAbsQoPT )  return false;
  if (m_maxAbsQoPT!=-9999. && std::fabs(qOverPT(*truth)) > m_maxAbsQoPT )  return false;
  if (m_isHadron           && ! isHadron(*truth) )                       return false;
  if (m_isPion             && ! isPion(*truth) )                         return false;

  if ( m_isFromTau || m_isFromB || m_isFromC || m_isFromHeavyFlav || m_isFromLightFlav ) {
    const xAOD::TruthParticle* truthParent = nullptr;
    if( m_isFromTau ) truthParent = getParent(truth, MC::TAU);
    if( m_isFromB ) truthParent = getParent(truth, MC::BQUARK);
    if( m_isFromC ) truthParent = getParent(truth, MC::CQUARK);
    if(m_isFromHeavyFlav || m_isFromLightFlav){
      const xAOD::TruthParticle* truthParentB = getParent(truth, MC::BQUARK);
      const xAOD::TruthParticle* truthParentC = getParent(truth, MC::CQUARK);
      truthParent = truthParentB ? truthParentB : (truthParentC ? truthParentC : nullptr);
    }
    if (m_isFromLightFlav && truthParent ) return false;
    if (not m_isFromLightFlav && ! truthParent) return false;  
    if (m_minParentPt!=-9999. && truthParent->pt() < m_minParentPt )           return false;
    if (m_maxParentPt!=-9999. && truthParent->pt() > m_maxParentPt )           return false;
  }

  return true;
}



