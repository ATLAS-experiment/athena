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
    public extends<asg::AsgService, ITrackAnalysisDefinitionSvc> {

public:
  using extends::extends;  // base class constructor
  virtual ~TrackAnalysisDefinitionSvc() = default;

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
  virtual float truthProbCut() const override { return m_truthProbCut.value(); };

  virtual const std::vector<float>& etaBins() const override { return m_etaBins; };
  virtual const std::vector<unsigned int>& minSilHits() const override { return m_minSilHits; };
  virtual const std::string& pileupSwitch() const override { return m_pileupSwitch; };
  virtual bool hasFullPileupTruth() const override { return m_hasFullPileupTruth.value(); };

  virtual bool plotTrackParameters() const override { return m_plotTrackParameters.value(); };
  virtual bool plotTrackMultiplicities() const override { return m_plotTrackMultiplicities.value(); };
  virtual bool plotEfficiencies() const override { return m_plotEfficiencies.value(); };
  virtual bool plotTechnicalEfficiencies() const override { return m_plotTechnicalEfficiencies.value(); };
  virtual bool plotResolutions() const override { return m_plotResolutions.value(); };
  virtual bool plotFakeRates() const override { return m_plotFakeRates.value(); };
  virtual bool unlinkedAsFakes() const override { return m_unlinkedAsFakes.value(); };
  virtual bool plotDuplicateRates() const override { return m_plotDuplicateRates.value(); };
  virtual bool plotHitsOnTracks() const override { return m_plotHitsOnTracks.value(); };
  virtual bool plotHitsOnTracksReference() const override { return m_plotHitsOnTracksReference.value(); };
  virtual bool plotHitsOnMatchedTracks() const override { return m_plotHitsOnMatchedTracks.value(); };
  virtual bool plotHitsOnFakeTracks() const override { return m_plotHitsOnFakeTracks.value(); };
  virtual bool plotVertexParameters() const override { return m_plotVertexParameters.value(); };
  virtual bool useSelectedVertexTracks() const override { return m_useSelectedVertexTracks.value(); };
  virtual bool plotOfflineElectrons() const override { return m_plotOfflineElectrons.value(); };
  virtual unsigned int resolutionMethod() const override;
  virtual bool isITk() const override { return m_isITk.value(); };

private:

  StringArrayProperty m_chainNames { this, "ChainNames", {}, "Vector of trigger chain names to process" }; 
  StringProperty m_dirName{ this, "DirName", "InDetTrackPerfMonPlots/", "Top level directory to write histograms into" };
  StringProperty m_subFolder { this, "SubFolder", "", "Subfolder to add for plots in. Used when working with multiple IDTPM tool instances and initialised by default to TrkAnaName/" }; 
  StringProperty m_trkAnaTag { this, "TrkAnaTag", "", "Track analysis tag name" }; 

  StringProperty m_testTypeStr { this, "TestType", "Offline", "Type of track collection to be used as test" }; 
  StringProperty m_refTypeStr { this, "RefType", "Truth", "Type of track collection to be used as reference" }; 

  bool m_useTrigger{}, m_useEFTrigger{}, m_useTruth{}, m_useOffline{};
  bool m_isTestTrigger{}, m_isTestEFTrigger{}, m_isTestTruth{}, m_isTestOffline{};
  bool m_isRefTrigger{}, m_isRefEFTrigger{}, m_isRefTruth{}, m_isRefOffline{};

  StringProperty m_testTag { this, "TestTag", "offl", "Short label for test track type, used in histo booking" }; 
  StringProperty m_refTag { this, "RefTag", "truth", "Short label for reference track type, used in histo booking" }; 

  StringProperty m_matchingType { this, "MatchingType", "DeltaRMatch", "Type of test-reference matching performed" }; 
  FloatProperty m_truthProbCut { this, "MatchingTruthProb", 0.5, "Minimal truthProbability for valid matching" };

  std::vector< std::string > m_configuredChains;

  FloatArrayProperty m_etaBins { this, "EtaBins", {}, "Eta bins for determination of reconstructable particle" };
  UnsignedIntegerArrayProperty m_minSilHits { this, "MinSilHits", {}, "Minimum number of Si hits for determination of reconstructable particle" };
  StringProperty m_pileupSwitch { this, "pileupSwitch", "HardScatter", "Type of truth particles to consider (HardScatter, PileUp, All)" }; 
  BooleanProperty m_hasFullPileupTruth { this, "hasFullPileupTruth", false, "Is full PileUp truth information available" };

  /// histogram properties
  BooleanProperty m_sortPlotsByChain { this, "sortPlotsByChain", false, "Save plots in <mainDir>/<chain>/<subDir/TrkAnaName>/... instead of the default <mainDir>/<subDir/TrkAnaName>/<chain>/..." };
  BooleanProperty m_plotTrackParameters { this, "plotTrackParameters", true, "Book/fill track parameters histograms" };
  BooleanProperty m_plotTrackMultiplicities { this, "plotTrackMultiplicities", true, "Book/fill track multiplicities histograms" };
  BooleanProperty m_plotEfficiencies { this, "plotEfficiencies", true, "Book/fill track efficiencies histograms" };
  BooleanProperty m_plotTechnicalEfficiencies { this, "plotTechnicalEfficiencies", true, "Book/fill track technical efficiencies histograms" };
  BooleanProperty m_plotResolutions { this, "plotResolutions", true, "Book/fill track resolutions histograms" };
  BooleanProperty m_plotFakeRates { this, "plotFakeRates", true, "Book/fill fake rate histograms" };
  BooleanProperty m_unlinkedAsFakes { this, "unlinkedAsFakes", false, "Consider non-truth-linked tracks as fakes" };
  BooleanProperty m_plotDuplicateRates { this, "plotDuplicateRates", false, "Book/fill duplicate rate histograms" };
  BooleanProperty m_plotHitsOnTracks { this, "plotHitsOnTracks", true, "Book/fill hits on tracks histograms" };
  BooleanProperty m_plotHitsOnTracksReference { this, "plotHitsOnTracksReference", false, "Book/fill hits on reference tracks histograms" };
  BooleanProperty m_plotHitsOnMatchedTracks { this, "plotHitsOnMatchedTracks", false, "Book/fill hits on matched tracks histograms" };
  BooleanProperty m_plotHitsOnFakeTracks { this, "plotHitsOnFakeTracks", false, "Book/fill hits on fake and unlinked tracks histograms" };
  BooleanProperty m_plotVertexParameters { this, "plotVertexParameters", true, "Book/fill vertex parameters histograms" };
  BooleanProperty m_useSelectedVertexTracks { this, "useSelectedVertexTracks", false, "Get only vertex-associated tracks which pass the track selection" };
  BooleanProperty m_plotOfflineElectrons { this, "plotOfflineElectrons", false, "Book/fill reference offline electrons histograms" };
  StringProperty m_resolMethod { this, "ResolutionMethod", "iterRMS", "Type of computation method for resolutions" };
  BooleanProperty m_isITk { this, "isITk", true, "Use ITk configuration for plots, etc." };
};

#endif // > !INDETTRACKPERFMON_TRACKANALYSISDEFINITIONSVC_H
