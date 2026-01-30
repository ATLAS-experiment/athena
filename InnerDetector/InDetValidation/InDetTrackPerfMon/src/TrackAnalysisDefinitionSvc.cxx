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

  ATH_MSG_DEBUG( "Initialising  using TEST = " << m_testTypeStr.value() <<
                 " && REFERENCE = " << m_refTypeStr.value() );

  /// setting flags
  m_isTestTrigger = m_testTypeStr.value().find("Trigger") != std::string::npos;
  m_isTestEFTrigger = m_testTypeStr.value().find("EFTrigger") != std::string::npos;
  m_isTestTruth   = m_testTypeStr.value().find("Truth") != std::string::npos;
  m_isTestOffline = m_testTypeStr.value().find("Offline") != std::string::npos;

  m_isRefTrigger = m_refTypeStr.value().find("Trigger") != std::string::npos;
  m_isRefEFTrigger = m_refTypeStr.value().find("EFTrigger") != std::string::npos;
  m_isRefTruth   = m_refTypeStr.value().find("Truth") != std::string::npos;
  m_isRefOffline = m_refTypeStr.value().find("Offline") != std::string::npos;

  m_useTrigger = m_isTestTrigger || m_isRefTrigger;
  m_useEFTrigger = m_isTestEFTrigger || m_isRefEFTrigger;
  m_useTruth   = m_isTestTruth || m_isRefTruth || m_matchingType.value().find("EFTruthMatch") != std::string::npos;;
  ATH_MSG_DEBUG( "USE TRUTH? " << m_useTruth );
  m_useOffline = m_isTestOffline || m_isRefOffline;

  /// Looping all requested chains && filling configured chains list (to be processed)
  if( m_doTrigNavigation ) {
    for( size_t ic=0 ; ic<m_chainNames.size() ; ic++ ) {
      ATH_MSG_DEBUG( "Input chain : " << m_chainNames[ic] );
      m_configuredChains.push_back( m_chainNames[ic] );
    }
  } else {
    /// Offline analysis (or EFtrigger)-> process only one "dummy chain" called "Offline"
    m_configuredChains.push_back( "Offline" );
  }

  /// sorting && removing duplicates from m_configuredChains
  std::sort( m_configuredChains.begin(), m_configuredChains.end() );
  m_configuredChains.erase( std::unique( m_configuredChains.begin(), 
                                         m_configuredChains.end() ), 
                            m_configuredChains.end() );

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
  std::string topDir( m_dirName );
  if( ! topDir.empty() ) topDir += "/";

  /// get "chainName/" || "" if empty
   if( ! chain.empty() ) chain += "/";

  /// get "subDir"
  std::string subDir( m_subFolder );
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

  return m_sortPlotsByChain.value() ?
         topDir + chain + subDir :
         topDir + subDir + chain;
}

/// ------------------------
/// --- resolutionMethod ---
/// ------------------------
unsigned int TrackAnalysisDefinitionSvc::resolutionMethod() const
{
  /// Defining map
  using methodMap_t = std::unordered_map<
      std::string, IDPVM::ResolutionHelper::methods >;
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
