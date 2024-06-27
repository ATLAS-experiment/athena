/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
  virtual std::string plotsFullDir( std::string chain="" ) const override;

  virtual bool useTrigger() const override { return m_useTrigger; }
  virtual bool useEFTrigger() const override { return m_useEFTrigger; }
  virtual bool useTruth() const override { return m_useTruth; }
  virtual bool useOffline() const override { return m_useOffline; }

  virtual bool isTestTrigger() const override { return m_isTestTrigger; }
  virtual bool isTestEFTrigger() const override { return m_isTestEFTrigger; }
  virtual bool isTestTruth() const override { return m_isTestTruth; }
  virtual bool isTestOffline() const override { return m_isTestOffline; }
  virtual bool isReferenceTrigger() const override { return m_isRefTrigger; }
  virtual bool isReferenceEFTrigger() const override { return m_isRefEFTrigger; }
  virtual bool isReferenceTruth() const override { return m_isRefTruth; }
  virtual bool isReferenceOffline() const override { return m_isRefOffline; }

  virtual const std::string& testType() const override { return m_testTypeStr.value(); };
  virtual const std::string& referenceType() const override { return m_refTypeStr.value(); };
  virtual const std::string& testTag() const override { return m_testTag.value(); };
  virtual const std::string& referenceTag() const override { return m_refTag.value(); };
  virtual const std::string& matchingType() const override { return m_matchingType.value(); };

  virtual bool plotTrackParameters() const override { return m_plotTrackParameters.value(); };
  virtual bool plotEfficiencies() const override { return m_plotEfficiencies.value(); };
  virtual bool plotResolutions() const override { return m_plotResolutions.value(); };
  virtual bool plotOfflineElectrons() const override { return m_plotOfflineElectrons.value(); };

private:

  StringArrayProperty m_chainNames { this, "ChainNames", {}, "Vector of trigger chain names to process" }; 
  StringProperty m_dirName{ this, "DirName", "InDetTrackPerfMonPlots/", "Top level directory to write histograms into" };
  StringProperty m_subFolder { this, "SubFolder", "", "Subfolder to add for plots in. Used when working with multiple IDTPM tool instances and initialised by default to TrkAnaName/" }; 
  StringProperty m_trkAnaTag { this, "TrkAnaTag", "", "Track analysis tag name" }; 

  StringProperty m_testTypeStr { this, "TestType", "Offline", "Type of track collection to be used as test" }; 
  StringProperty m_refTypeStr { this, "RefType", "Truth", "Type of track collection to be used as reference" }; 
  bool m_useTrigger, m_useEFTrigger, m_useTruth, m_useOffline;
  bool m_isTestTrigger, m_isTestEFTrigger, m_isTestTruth, m_isTestOffline;
  bool m_isRefTrigger, m_isRefEFTrigger, m_isRefTruth, m_isRefOffline;

  StringProperty m_testTag { this, "TestTag", "offl", "Short label for test track type, used in histo booking" }; 
  StringProperty m_refTag { this, "RefTag", "truth", "Short label for reference track type, used in histo booking" }; 

  StringProperty m_matchingType { this, "MatchingType", "DeltaRMatch", "Type of test-reference matching performed" }; 

  std::vector< std::string > m_configuredChains;

  /// histogram properties
  BooleanProperty m_sortPlotsByChain { this, "sortPlotsByChain", false, "Save plots in <mainDir>/<chain>/<subDir/TrkAnaName>/... instead of the default <mainDir>/<subDir/TrkAnaName>/<chain>/..." };
  BooleanProperty m_plotTrackParameters { this, "plotTrackParameters", true, "Book/fill track parameters histograms" };
  BooleanProperty m_plotEfficiencies { this, "plotEfficiencies", true, "Book/fill track parameters histograms" };
  BooleanProperty m_plotResolutions { this, "plotResolutions", true, "Book/fill track resolutions histograms" };
  BooleanProperty m_plotOfflineElectrons { this, "plotOfflineElectrons", false, "Book/fill reference offline electrons histograms" };
 
};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
