/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackAnalysisDefinitionSvc.cxx
 * @author marco aparo
 * @date 19 June 2023
**/

/// local includes
#include "TrackAnalysisDefinitionSvc.h"

/// STL includes 
#include <algorithm>

/// -------------------
/// --- Constructor ---
/// -------------------
TrackAnalysisDefinitionSvc::TrackAnalysisDefinitionSvc( const std::string& name, ISvcLocator* pSvcLocator ) :
    AsgService( name, pSvcLocator ), 
    m_useTrigger( false ), m_useTruth( false ), m_useOffline( false ),
    m_isTestTrigger( false ), m_isTestTruth( false ), m_isTestOffline( false ),
    m_isRefTrigger( false ), m_isRefTruth( false ), m_isRefOffline( false )
{
  declareServiceInterface< ITrackAnalysisDefinitionSvc >();
}

/// ------------------
/// --- Destructor ---
/// ------------------
TrackAnalysisDefinitionSvc::~TrackAnalysisDefinitionSvc() = default;

/// ------------------
/// --- initialize ---
/// ------------------
StatusCode TrackAnalysisDefinitionSvc::initialize()
{

  ATH_MSG_INFO( "Initialising  using TEST = " << m_testTypeStr.value() <<
                " and REFERENCE = " << m_refTypeStr.value() );

  /// setting flags
  m_isTestTrigger = m_testTypeStr.value().find("Trigger") != std::string::npos;
  m_isTestTruth   = m_testTypeStr.value().find("Truth") != std::string::npos;
  m_isTestOffline = m_testTypeStr.value().find("Offline") != std::string::npos;

  m_isRefTrigger = m_refTypeStr.value().find("Trigger") != std::string::npos;
  m_isRefTruth   = m_refTypeStr.value().find("Truth") != std::string::npos;
  m_isRefOffline = m_refTypeStr.value().find("Offline") != std::string::npos;

  m_useTrigger = m_isTestTrigger or m_isRefTrigger;
  m_useTruth   = m_isTestTruth or m_isRefTruth;
  m_useOffline = m_isTestOffline or m_isRefOffline;

  /// Looping all requested chains and filling configured chains list (to be processed)
  if( m_useTrigger ) {
    for( size_t ic=0 ; ic<m_chainNames.size() ; ic++ ) {
      ATH_MSG_DEBUG( "Input chain : " << m_chainNames[ic] );
      m_configuredChains.push_back( m_chainNames[ic] );
    }
  } else {
    /// Offline analysis -> process only one "dummy chain" called "Offline"
    m_configuredChains.push_back( "Offline" );
  }

  /// sorting and removing duplicates from m_configuredChains
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
