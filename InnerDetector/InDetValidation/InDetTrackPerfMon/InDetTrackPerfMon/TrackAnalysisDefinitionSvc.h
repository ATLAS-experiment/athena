/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
#define INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H

/**
 * @file TrackAnalysisDefinitionSvc.h
 * AthService to hold (and propagate) the basic properties
 * of each defined TrackAnalysis && of their hisotgrams
 * @author marco aparo
 * @date 19 June 2023
**/

/// Athena includes
#include "AsgServices/AsgService.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "InDetTrackPerfMon/TrackAnalysisDefinition.h"

/// STL includes
#include <string>
#include <vector>

class TrackAnalysisDefinitionSvc final :
    public extends<asg::AsgService, ITrackAnalysisDefinitionSvc> {

public:
  using extends::extends;  // base class constructor
  virtual ~TrackAnalysisDefinitionSvc() = default;

  virtual StatusCode initialize() override final;

  virtual StatusCode finalize() override final;

  virtual const ITrackAnalysisDefinition& get() const { return m_def; }  
  
  /// TrackAnalysisDefinition delegates ...
  virtual const std::vector< std::string >& configuredChains() const override { return m_configuredChains; }
  virtual const std::string& subFolder() const override { return m_def.subFolder(); };
  /// why do the accessors have different names to the actual variables ?????
  virtual const std::string& anaTag()    const override { return m_def.anaTag(); };

  virtual bool useTrigger()   const override { return m_def.useTrigger(); }
  virtual bool useEFTrigger() const override { return m_def.useEFTrigger(); }
  virtual bool useTruth()     const override { return m_def.useTruth(); }
  virtual bool useOffline()   const override { return m_def.useOffline(); }
  virtual bool doTrigNavigation() const override { return m_def.doTrigNavigation(); }

  virtual bool isTestTrigger()   const override { return m_def.isTestTrigger(); }
  virtual bool isTestEFTrigger() const override { return m_def.isTestEFTrigger(); }
  virtual bool isTestTruth()     const override { return m_def.isTestTruth(); }
  virtual bool isTestOffline()   const override { return m_def.isTestOffline(); }

  /// why do the accessors have different names to the actual variables ?? AAARGH  
  virtual bool isReferenceTrigger()   const override { return m_def.isReferenceTrigger(); }
  virtual bool isReferenceEFTrigger() const override { return m_def.isReferenceEFTrigger(); }
  virtual bool isReferenceTruth()     const override { return m_def.isReferenceTruth(); }
  virtual bool isReferenceOffline()   const override { return m_def.isReferenceOffline(); }

  virtual const std::string& testType()      const override { return m_def.testType(); };
  virtual const std::string& referenceType() const override { return m_def.referenceType(); };
  virtual const std::string& testTag()       const override { return m_def.testTag(); };
  virtual const std::string& referenceTag()  const override { return m_def.referenceTag(); };
  virtual const std::string& matchingType()  const override { return m_def.matchingType(); };
  virtual float truthProbCut()               const override { return m_def.truthProbCut(); };

  virtual const std::vector<float>& etaBins()           const override { return m_def.etaBins(); };
  virtual const std::vector<unsigned int>& minSilHits() const override { return m_def.minSilHits(); };
  virtual const std::string& pileupSwitch()             const override { return m_def.pileupSwitch(); };
  virtual bool hasFullPileupTruth()                     const override { return m_def.hasFullPileupTruth(); };


  /// TrackAnalysisDefniitionSvc specific functions ...
  virtual std::string plotsFullDir( std::string chain="" ) const override;
  
  virtual bool plotTrackParameters() const override { return m_plotTrackParameters.value(); };
  virtual bool plotTrackParametersErrors() const override { return m_plotTrackParametersErrors.value(); };
  virtual bool plotTrackMultiplicities() const override { return m_plotTrackMultiplicities.value(); };
  virtual bool plotEfficiencies() const override { return m_plotEfficiencies.value(); };
  virtual bool plotTechnicalEfficiencies() const override { return m_plotTechnicalEfficiencies.value(); };
  virtual bool plotResolutions() const override { return m_plotResolutions.value(); };
  virtual bool plotFakeRates() const override { return m_plotFakeRates.value(); };
  virtual bool unlinkedAsFakes() const override { return m_unlinkedAsFakes.value(); };
  virtual bool plotDuplicateRates() const override { return m_plotDuplicateRates.value(); };
  virtual bool plotHitsOnTracks() const override { return m_plotHitsOnTracks.value(); };
  virtual bool plotHitsOnTracksExpert() const override { return m_plotHitsOnTracksExpert.value(); };
  virtual bool plotHitsOnTracksReference() const override { return m_plotHitsOnTracksReference.value(); };
  virtual bool plotHitsOnMatchedTracks() const override { return m_plotHitsOnMatchedTracks.value(); };
  virtual bool plotHitsOnFakeTracks() const override { return m_plotHitsOnFakeTracks.value(); };
  virtual bool plotVertexParameters() const override { return m_plotVertexParameters.value(); };
  virtual bool useSelectedVertexTracks() const override { return m_useSelectedVertexTracks.value(); };
  virtual bool plotOfflineElectrons() const override { return m_plotOfflineElectrons.value(); };
  virtual bool plotTracksInJets() const override { return m_plotTracksInJets.value(); };
  virtual unsigned int resolutionMethod() const override;
  virtual bool isITk() const override { return m_isITk.value(); };

private:

  StringArrayProperty m_chainNames_prop { this, "ChainNames", {}, "Vector of trigger chain names to process" }; 
  StringProperty      m_dirName_prop    { this, "DirName", "InDetTrackPerfMonPlots/", "Top level directory to write histograms into" };
  StringProperty      m_subFolder_prop  { this, "SubFolder", "", "Subfolder to add for plots in. Used when working with multiple IDTPM tool instances && initialised by default to TrkAnaName/" }; 
  StringProperty      m_trkAnaTag_prop  { this, "TrkAnaTag", "", "Track analysis tag name" }; 

  StringProperty      m_testTypeStr_prop      { this, "TestType", "Offline", "Type of track collection to be used as test" }; 
  StringProperty      m_refTypeStr_prop       { this, "RefType", "Truth", "Type of track collection to be used as reference" }; 

  /// the trig navigation should be automatcially conmfigured whenere we want to run an analysis using
  /// any trigger data, so for instance if any collection name, or a trigger instance
  /// includes HLT_ then the navigation should be required
  BooleanProperty     m_doTrigNavigation_prop { this, "doTrigNavigation", false, "Run Trigger Navigation monitoring" };

  bool m_useTrigger{},    m_useEFTrigger{}, m_useTruth{}, m_useOffline{};
  bool m_isTestTrigger{}, m_isTestEFTrigger{}, m_isTestTruth{}, m_isTestOffline{};
  bool m_isRefTrigger{}, m_isRefEFTrigger{}, m_isRefTruth{}, m_isRefOffline{};

  StringProperty m_testTag_prop { this, "TestTag", "offl", "Short label for test track type, used in histo booking" }; 
  StringProperty m_refTag_prop  { this, "RefTag", "truth", "Short label for reference track type, used in histo booking" }; 

  StringProperty m_matchingType_prop { this, "MatchingType", "DeltaRMatch", "Type of test-reference matching performed" }; 
  FloatProperty  m_truthProbCut_prop { this, "MatchingTruthProb", 0.5, "Minimal truthProbability for valid matching" };

  std::vector< std::string > m_configuredChains;

  FloatArrayProperty           m_etaBins_prop       { this, "EtaBins", {}, "Eta bins for determination of reconstructable particle" };
  UnsignedIntegerArrayProperty m_minSilHits_prop    { this, "MinSilHits", {}, "Minimum number of Si hits for determination of reconstructable particle" };
  StringProperty               m_pileupSwitch_prop  { this, "pileupSwitch", "HardScatter", "Type of truth particles to consider (HardScatter, PileUp, All)" }; 
  BooleanProperty              m_hasFullPileupTruth_prop { this, "hasFullPileupTruth", false, "Is full PileUp truth information available" };

  /// the container for the actual parameters that are needed wlsewhere
  /// since this is essentially the only purpose of this class to distribute
  /// these parameters, there ia absolutely nbo need whatsoever to actually
  /// have this class implemented as a service
  TrackAnalysisDefinition m_def;
  
  /// histogram properties
  BooleanProperty m_sortPlotsByChain { this, "sortPlotsByChain", false, "Save plots in <mainDir>/<chain>/<subDir/TrkAnaName>/... instead of the default <mainDir>/<subDir/TrkAnaName>/<chain>/..." };
  BooleanProperty m_plotTrackParameters { this, "plotTrackParameters", true, "Book/fill track parameters histograms" };
  BooleanProperty m_plotTrackParametersErrors { this, "plotTrackParametersErrors", false, "Book/fill track parameters errors histograms" };
  BooleanProperty m_plotTrackMultiplicities { this, "plotTrackMultiplicities", true, "Book/fill track multiplicities histograms" };
  BooleanProperty m_plotEfficiencies { this, "plotEfficiencies", true, "Book/fill track efficiencies histograms" };
  BooleanProperty m_plotTechnicalEfficiencies { this, "plotTechnicalEfficiencies", true, "Book/fill track technical efficiencies histograms" };
  BooleanProperty m_plotResolutions { this, "plotResolutions", true, "Book/fill track resolutions histograms" };
  BooleanProperty m_plotFakeRates { this, "plotFakeRates", true, "Book/fill fake rate histograms" };
  BooleanProperty m_unlinkedAsFakes { this, "unlinkedAsFakes", false, "Consider non-truth-linked tracks as fakes" };
  BooleanProperty m_plotDuplicateRates { this, "plotDuplicateRates", false, "Book/fill duplicate rate histograms" };
  BooleanProperty m_plotHitsOnTracks { this, "plotHitsOnTracks", true, "Book/fill hits on tracks histograms" };
  BooleanProperty m_plotHitsOnTracksExpert { this, "plotHitsOnTracksExpert", true, "Book/fill hits on tracks detailed histograms" };
  BooleanProperty m_plotHitsOnTracksReference { this, "plotHitsOnTracksReference", false, "Book/fill hits on reference tracks histograms" };
  BooleanProperty m_plotHitsOnMatchedTracks { this, "plotHitsOnMatchedTracks", false, "Book/fill hits on matched tracks histograms" };
  BooleanProperty m_plotHitsOnFakeTracks { this, "plotHitsOnFakeTracks", false, "Book/fill hits on fake && unlinked tracks histograms" };
  BooleanProperty m_plotVertexParameters { this, "plotVertexParameters", true, "Book/fill vertex parameters histograms" };
  BooleanProperty m_useSelectedVertexTracks { this, "useSelectedVertexTracks", false, "Get only vertex-associated tracks which pass the track selection" };
  BooleanProperty m_plotOfflineElectrons { this, "plotOfflineElectrons", false, "Book/fill reference offline electrons histograms" };
  BooleanProperty m_plotTracksInJets { this, "plotTracksInJets", false, "plot tracks in jets" };
  StringProperty  m_resolMethod { this, "ResolutionMethod", "iterRMS", "Type of computation method for resolutions" };
  BooleanProperty m_isITk { this, "isITk", true, "Use ITk configuration for plots, etc." };
};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
