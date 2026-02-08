/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackAnalysisDefinitionSvc.cxx
 * @author marco aparo
 * @date 19 June 2023
**/

/// local includes
#include "InDetTrackPerfMon/TrackAnalysisDefinitionSvc.h"


/// STL includes 
#include <algorithm>
#include <memory>
#include <unordered_map>

/// Athena includes
#include "InDetPhysValMonitoring/ResolutionHelper.h"


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode TrackAnalysisDefinitionSvc::initialize()
{

  ATH_MSG_DEBUG( "Initialising  using TEST = " << m_testTypeStr_prop.value() <<
                 " && REFERENCE = " << m_refTypeStr_prop.value() );


  /// setting the default flags
  m_def.m_chainNames = m_chainNames_prop;
  m_def.m_dirName    = m_dirName_prop;
  m_def.m_subFolder  = m_subFolder_prop;
  m_def.m_trkAnaTag  = m_trkAnaTag_prop;

  m_def.m_testTypeStr = m_testTypeStr_prop;
  m_def.m_refTypeStr  = m_refTypeStr_prop;

  m_def.m_doTrigNavigation = m_doTrigNavigation_prop;

  m_def.m_testTag = m_testTag_prop;
  m_def.m_refTag  = m_refTag_prop;

  m_def.m_matchingType = m_matchingType_prop;
  m_def.m_truthProbCut = m_truthProbCut_prop;

  m_def.m_etaBins       = m_etaBins_prop;
  m_def.m_minSilHits    = m_minSilHits_prop;
  m_def.m_pileupSwitch  = m_pileupSwitch_prop;
  m_def.m_hasFullPileupTruth = m_hasFullPileupTruth_prop;
    
  /// setting flags
  /// why not define contains( s0, s1  ) return s0.find(s1) !=-std::npos  ? 
  /// would make things much neater
  /// NB: since the m_def values are not being set here, we should
  ///     really be using the m_def getters rather thaj the raw values themselves
  m_def.m_isTestTrigger   = m_def.m_testTypeStr.find("Trigger") != std::string::npos;
  m_def.m_isTestEFTrigger = m_def.m_testTypeStr.find("EFTrigger") != std::string::npos;
  m_def.m_isTestTruth     = m_def.m_testTypeStr.find("Truth") != std::string::npos;
  m_def.m_isTestOffline   = m_def.m_testTypeStr.find("Offline") != std::string::npos;

  m_def.m_isRefTrigger   = m_def.m_refTypeStr.find("Trigger") != std::string::npos;
  m_def.m_isRefEFTrigger = m_def.m_refTypeStr.find("EFTrigger") != std::string::npos;
  m_def.m_isRefTruth     = m_def.m_refTypeStr.find("Truth") != std::string::npos;
  m_def.m_isRefOffline   = m_def.m_refTypeStr.find("Offline") != std::string::npos;

  m_def.m_useTrigger   = m_def.m_isTestTrigger   || m_def.m_isRefTrigger;
  m_def.m_useEFTrigger = m_def.m_isTestEFTrigger || m_def.m_isRefEFTrigger;
  m_def.m_useTruth     = m_def.m_isTestTruth || m_def.m_isRefTruth || m_def.m_matchingType.find("EFTruthMatch") != std::string::npos;;

  
  ATH_MSG_DEBUG( "USE TRUTH? " << m_def.m_useTruth );
  m_def.m_useOffline = m_def.m_isTestOffline || m_def.m_isRefOffline;

  /// Looping all requested chains && filling configured chains list (to be processed)
  /// this isn;t how we want to do it, we don't want lots of different classes
  /// having lists of chains to be processed, and we don;t what it controlled by
  /// algorithms themselves, loopiong over chains based on some list of
  /// chains in some python configured class, because we need to set up
  /// a GenericMonTool for every analysis in the python, so we need the
  /// list to be determined once-and-for-all in the python, and then passed
  /// into the class which will configure the tools. Then that class makes sure
  /// that there are the right number of tools, and every chains, has only
  /// the one tool, and nothing else should know about any of the otther
  /// chains. If analyses can share tools, then that shoul be up to the
  /// the master algorithm, not up to the tools or the chains themselves - 
  /// they should not even be aware of whether the tool they are using belongs 
  /// to them, or is shared with anyone else

  if ( m_def.m_chainNames.size()>1 ) std::cerr << "oh dear, too many will chains configured" << std::endl;
  
  if( m_def.m_doTrigNavigation ) {
    for( size_t ic=0 ; ic<m_def.m_chainNames.size() ; ic++ ) {
      ATH_MSG_DEBUG( "Input chain : " << m_def.m_chainNames[ic] );
      m_def.m_configuredChains.push_back( m_def.m_chainNames[ic] );
    }
  } else {
    /// Offline analysis (or EFtrigger)-> process only one "dummy chain" called "Offline"
    m_def.m_configuredChains.push_back( "Offline" );
  }

  /// sorting && removing duplicates from m_configuredChains
  /// what ??? why should there be duplicates ? We can have
  /// the same chains processed with different analysese, so this
  /// should not be done like this at all
  std::sort( m_def.m_configuredChains.begin(), m_def.m_configuredChains.end() );
  m_def.m_configuredChains.erase( std::unique( m_def.m_configuredChains.begin(), 
					       m_def.m_configuredChains.end() ), 
				  m_def.m_configuredChains.end() );
  
  return StatusCode::SUCCESS;
}

/// ----------------
/// --- finalize ---
/// ----------------
StatusCode TrackAnalysisDefinitionSvc::finalize() {
  return StatusCode::SUCCESS;
}

/// --------------------
/// --- plotsFullDir ---
/// --------------------
std::string TrackAnalysisDefinitionSvc::plotsFullDir( std::string chain ) const
{
  /// get "topDir/" || "" if empty
  std::string topDir( m_def.m_dirName );
  if( ! topDir.empty() ) topDir += "/";

  /// get "chainName/" || "" if empty
   if( ! chain.empty() ) chain += "/";

  /// get "subDir"
  std::string subDir( m_def.m_subFolder );
  if( subDir.empty() ) {
    ATH_MSG_WARNING( "Empty plots sub-directory" );
  } else  {
    /// reduce: "/subDir" -> "subDir"
    if( subDir[0] == '/' ) {
      subDir.erase( subDir.begin() );
    }
    /// add a slash: "subDir" -> "subDir/"
    if( subDir.back() != '/' ) subDir += "/";
  }

  std::string output = output = topDir + subDir + chain;

  /// what is going on here ???
  if ( m_sortPlotsByChain.value() ) output = topDir + chain + subDir;

  return output;
  
}

/// ------------------------
/// --- resolutionMethod ---
/// ------------------------
unsigned int TrackAnalysisDefinitionSvc::resolutionMethod() const
{
  /// Defining map
  /// so we have amap like this for the resolution helpers, but not for the matchers ?
  /// and we have the nasty matcher factory in the python ? Insane
  using methodMap_t = std::unordered_map<std::string, IDPVM::ResolutionHelper::methods>;
  methodMap_t methodMap = {
    { "iterRMS"         , IDPVM::ResolutionHelper::iterRMS_convergence },
    { "gaussFit"        , IDPVM::ResolutionHelper::Gauss_fit },
    { "iterRMSgaussFit" , IDPVM::ResolutionHelper::fusion_iterRMS_Gaussfit },
    { "iterGaussFit"    , IDPVM::ResolutionHelper::iterGaussFit_convergence }
  };

  methodMap_t::const_iterator mitr = methodMap.find( m_resolMethod.value() );
  if( mitr == methodMap.end() ) {
    ATH_MSG_DEBUG( "Method " << m_resolMethod.value() <<
                   " ! found. Using iterRMS by default." );
    return IDPVM::ResolutionHelper::iterRMS_convergence;
  }
  return mitr->second;
}
