/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISDEFINITION_H
#define INDETTRACKPERFMON_TRACKANALYSISDEFINITION_H

/**
 * @file TrackAnalysisDefinitionSvc.h
 * AthService to hold (and propagate) the basic properties
 * of each defined TrackAnalysis && of their hisotgrams
 * @author marco aparo and m sutton
 * @date 19 June 2023
**/

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinition.h"

/// STL includes
#include <string>
#include <vector>

class TrackAnalysisDefinitionSvc;



class TrackAnalysisDefinition : public ITrackAnalysisDefinition {

public:

  friend class TrackAnalysisDefinitionSvc;
  
  TrackAnalysisDefinition();
  ~TrackAnalysisDefinition();

  /// need to add setters for all these functions so that we can use the class by itself,
  /// rather than be forced to configure it as a "service"
  
  virtual const std::vector<std::string>& configuredChains() const  { return m_configuredChains; }
  virtual const std::string& subFolder()                     const  { return m_subFolder; };
  virtual const std::string& anaTag()                        const  { return m_trkAnaTag; };

  virtual bool useTrigger()       const  { return m_useTrigger; }
  virtual bool useEFTrigger()     const  { return m_useEFTrigger; }
  virtual bool useTruth()         const  { return m_useTruth; }
  virtual bool useOffline()       const  { return m_useOffline; }
  virtual bool doTrigNavigation() const  { return m_doTrigNavigation; }

  virtual bool isTestTrigger()    const  { return m_isTestTrigger; }
  virtual bool isTestEFTrigger()  const  { return m_isTestEFTrigger; }
  virtual bool isTestTruth()      const  { return m_isTestTruth; }
  virtual bool isTestOffline()    const  { return m_isTestOffline; }
  virtual bool isReferenceTrigger()   const  { return m_isRefTrigger; }
  virtual bool isReferenceEFTrigger() const  { return m_isRefEFTrigger; }
  virtual bool isReferenceTruth()     const  { return m_isRefTruth; }
  virtual bool isReferenceOffline()   const  { return m_isRefOffline; }

  virtual const std::string& testType()      const  { return m_testTypeStr; };
  virtual const std::string& referenceType() const  { return m_refTypeStr; };
  virtual const std::string& testTag()       const  { return m_testTag; };
  virtual const std::string& referenceTag()  const  { return m_refTag; };
  virtual const std::string& matchingType()  const  { return m_matchingType; };
  virtual float truthProbCut()               const  { return m_truthProbCut; };

  virtual const std::vector<float>& etaBins()           const  { return m_etaBins; };
  virtual const std::vector<unsigned int>& minSilHits() const  { return m_minSilHits; };
  virtual const std::string& pileupSwitch()             const  { return m_pileupSwitch; };
  virtual bool hasFullPileupTruth()                     const  { return m_hasFullPileupTruth; };

protected:

  std::vector<std::string> m_chainNames;

  std::string m_dirName;
  std::string m_subFolder;
  std::string m_trkAnaTag;

  std::string m_testTypeStr;
  std::string m_refTypeStr;
  
  bool  m_doTrigNavigation;

  /// we really, really, really, should not need, nor
  /// have all these redundent flags - it is very poor 
  bool m_useTrigger;
  bool m_useEFTrigger;
  bool m_useTruth;
  bool m_useOffline;
  bool m_isTestTrigger;
  bool m_isTestEFTrigger;
  bool m_isTestTruth;
  bool m_isTestOffline;;
  bool m_isRefTrigger;
  bool m_isRefEFTrigger;
  bool m_isRefTruth;
  bool m_isRefOffline;

  
  std::string    m_testTag;
  std::string    m_refTag;

  std::string    m_matchingType;
  float          m_truthProbCut;

  std::vector<std::string> m_configuredChains;

  std::vector<float>         m_etaBins;
  std::vector<unsigned int>  m_minSilHits;

  std::string   m_pileupSwitch;
  bool          m_hasFullPileupTruth;

};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
