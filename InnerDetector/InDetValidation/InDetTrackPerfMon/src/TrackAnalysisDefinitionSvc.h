/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
#define INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H

/**
 * @file TrackAnalysisDefinitionSvc.h
 * AthService to hold (and propagate) the basic properties
 * of each defined TrackAnalysis and of their hisotgrams
 * @author marco aparo
 * @date 19 June 2023
**/

/// Athena includes
#include "AsgServices/AsgService.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"

/// STL includes
#include <string>
#include <vector>
#include <memory>

class TrackAnalysisDefinitionSvc final :
    public asg::AsgService,
    virtual public ITrackAnalysisDefinitionSvc {

public:

  TrackAnalysisDefinitionSvc( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrackAnalysisDefinitionSvc();

  virtual StatusCode initialize() override final;

  virtual StatusCode finalize() override final;

  virtual const std::vector< std::string >& configuredChains() const override { return m_configuredChains; }
  virtual const std::string& subFolder() const override { return m_subFolder; };
  virtual const std::string& anaTag() const override { return m_trkAnaTag; };

  virtual bool useTrigger() const override { return m_useTrigger; }
  virtual bool useTruth() const override { return m_useTruth; }
  virtual bool useOffline() const override { return m_useOffline; }

  virtual bool isTestTrigger() const override { return m_isTestTrigger; }
  virtual bool isTestTruth() const override { return m_isTestTruth; }
  virtual bool isTestOffline() const override { return m_isTestOffline; }
  virtual bool isReferenceTrigger() const override { return m_isRefTrigger; }
  virtual bool isReferenceTruth() const override { return m_isRefTruth; }
  virtual bool isReferenceOffline() const override { return m_isRefOffline; }

  virtual const std::string& testType() const override { return m_testTypeStr.value(); };
  virtual const std::string& referenceType() const override { return m_refTypeStr.value(); };
  virtual const std::string& testTag() const override { return m_testTag.value(); };
  virtual const std::string& referenceTag() const override { return m_refTag.value(); };
  virtual const std::string& matchingType() const override { return m_matchingType.value(); };

  virtual bool doTrackParameters() const override { return m_doTrackParameters.value(); };
  virtual bool doEfficiencies() const override { return m_doEfficiencies.value(); };
  virtual bool doOfflineElectrons() const override { return m_doOfflineElectrons.value(); };

private:

  StringArrayProperty m_chainNames { this, "ChainNames", {}, "Vector of trigger chain names to process" }; 
  StringProperty m_subFolder { this, "SubFolder", "", "Subfolder to add for plots if desired. Used when working with multiple IDTPM tool instances" }; 
  StringProperty m_trkAnaTag { this, "TrkAnaTag", "", "Track analysis tag name" }; 

  StringProperty m_testTypeStr { this, "TestType", "Offline", "Type of track collection to be used as test" }; 
  StringProperty m_refTypeStr { this, "RefType", "Truth", "Type of track collection to be used as reference" }; 
  bool m_useTrigger, m_useTruth, m_useOffline;
  bool m_isTestTrigger, m_isTestTruth, m_isTestOffline;
  bool m_isRefTrigger, m_isRefTruth, m_isRefOffline;

  StringProperty m_testTag { this, "TestTag", "offl", "Short lable for test track type, used in histo booking" }; 
  StringProperty m_refTag { this, "RefTag", "truth", "Short lable for reference track type, used in histo booking" }; 

  StringProperty m_matchingType { this, "MatchingType", "DeltaRMatch", "Type of test-reference matching performed" }; 

  std::vector< std::string > m_configuredChains;

  /// histogram properties
  BooleanProperty m_doTrackParameters { this, "doTrackParameters", true, "Book/fill track parameters histgrams" };
  BooleanProperty m_doEfficiencies { this, "doEfficiencies", true, "Book/fill track parameters histgrams" };
  BooleanProperty m_doOfflineElectrons { this, "doOfflineElectrons", false, "Book/fill reference offline electrons histgrams" };
 
};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
