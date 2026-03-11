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

  virtual bool isTestTrigger()        const  { return m_isTestTrigger; }
  virtual bool isTestEFTrigger()      const  { return m_isTestEFTrigger; }
  virtual bool isTestTruth()          const  { return m_isTestTruth; }
  virtual bool isTestOffline()        const  { return m_isTestOffline; }
  virtual bool isReferenceTrigger()   const  { return m_isRefTrigger; }
  virtual bool isReferenceEFTrigger() const  { return m_isRefEFTrigger; }
  virtual bool isReferenceTruth()     const  { return m_isRefTruth; }
  virtual bool isReferenceOffline()   const  { return m_isRefOffline; }

  /// all this type and tag information, and the corresponding isTestOffline stuff
  /// is way more complicated than it should be and makes development
  /// really fragile
  virtual const std::string& testType()      const  { return m_testTypeStr; };
  virtual const std::string& referenceType() const  { return m_refTypeStr; };
  virtual const std::string& testTag()       const  { return m_testTag; };
  virtual const std::string& referenceTag()  const  { return m_refTag; };
  virtual const std::string& matchingType()  const  { return m_matchingType; };

  virtual const std::string& testCollection()       const  { return m_testCollection; };
  virtual const std::string& referenceCollection()  const  { return m_refCollection; };


  virtual float truthProbCut()               const  { return m_truthProbCut; };

  virtual const std::vector<float>& etaBins()           const  { return m_etaBins; };
  virtual const std::vector<unsigned int>& minSilHits() const  { return m_minSilHits; };
  virtual const std::string& pileupSwitch()             const  { return m_pileupSwitch; };
  virtual bool hasFullPileupTruth()                     const  { return m_hasFullPileupTruth; };

  /// setters - for use where we don't want to initialise as part of the DefinitionSvc
  
  virtual void setConfiguredChains( const std::vector<std::string>& value ) {   m_configuredChains = value; }
  virtual void setSubFolder( const std::string& value )  {  m_subFolder = value; }  
  virtual void    setAnaTag( const std::string& value )  {  m_trkAnaTag = value; }

  virtual void       setUseTrigger( bool b )  { m_useTrigger = b; }
  virtual void     setUseEFTrigger( bool b )  { m_useEFTrigger = b; }
  virtual void         setUseTruth( bool b )  { m_useTruth = b; }
  virtual void       setUseOffline( bool b )  { m_useOffline = b; }
  virtual void setDoTrigNavigation( bool b )  { m_doTrigNavigation = b; }
  
  virtual void      setIsTestTrigger( bool b )   { m_isTestTrigger = b; }
  virtual void    setIsTestEFTrigger( bool b )   { m_isTestEFTrigger = b; }
  virtual void        setIsTestTruth( bool b )   { m_isTestTruth = b; }
  virtual void      setIsTestOffline( bool b )   { m_isTestOffline = b; }
  virtual void setIsReferenceTrigger( bool b )   { m_isRefTrigger = b; }
  virtual void setIsReferenceEFTrigger( bool b ) { m_isRefEFTrigger = b ; } /// there is a better way to do all these
  virtual void   setIsReferenceTruth( bool b )   { m_isRefTruth   = b; }
  virtual void setIsReferenceOffline( bool b )   { m_isRefOffline = b; }

  virtual void      setTestType( const std::string& s )  { m_testTypeStr = s; }
  virtual void setReferenceType( const std::string& s )  { m_refTypeStr = s; }

  virtual void      setTestTag( const std::string& s )  { m_testTag = s; }
  virtual void setReferenceTag( const std::string& s )  { m_refTag  = s; }

  virtual void setMatchingType( const std::string& s )  { m_matchingType = s; }

  virtual void      setTestCollection( const std::string& s )  { m_testCollection = s; }
  virtual void setReferenceCollection( const std::string& s )  { m_refCollection  = s; }

  virtual void setTruthProbCut( float f ) { m_truthProbCut = f; }

  virtual void           setEtaBins( const std::vector<float>& v )  { m_etaBins = v; }
  virtual void setMinSilHits( const std::vector<unsigned int>& v )  { m_minSilHits = v; }
  virtual void             setPileupSwitch( const std::string& s )  { m_pileupSwitch = s; }
  virtual void                     setHasFullPileupTruth( bool b )  { m_hasFullPileupTruth = b; }
   
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

  std::string    m_testCollection;
  std::string    m_refCollection;

  std::string    m_matchingType;
  float          m_truthProbCut;

  std::vector<std::string> m_configuredChains;

  std::vector<float>         m_etaBins;
  std::vector<unsigned int>  m_minSilHits;

  std::string   m_pileupSwitch;
  bool          m_hasFullPileupTruth;

};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
