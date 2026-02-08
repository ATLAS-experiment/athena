/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TrackAnalysisDefinition.cxx
 * @author mark sutton
 * @date 12 February 2026
**/

#include "InDetTrackPerfMon/TrackAnalysisDefinition.h"


TrackAnalysisDefinition::TrackAnalysisDefinition() :
  m_chainNames(),  m_dirName(),  m_subFolder(),  m_trkAnaTag(),
  m_testTypeStr(), m_refTypeStr(),  m_doTrigNavigation(false),
  m_useTrigger(false),    m_useEFTrigger(false),    m_useTruth(false),    m_useOffline(false),
  m_isTestTrigger(false), m_isTestEFTrigger(false), m_isTestTruth(false), m_isTestOffline(false),
  m_isRefTrigger(false),  m_isRefEFTrigger(false),  m_isRefTruth(false),  m_isRefOffline(false),
  m_testTag(),  m_refTag(),
  m_matchingType(),  m_truthProbCut(0),
  m_configuredChains(),
  m_etaBins(),
  m_pileupSwitch(),  m_hasFullPileupTruth(false)
{ }


TrackAnalysisDefinition::~TrackAnalysisDefinition() { } 

