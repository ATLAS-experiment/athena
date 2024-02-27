/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RIVET_I_H
#define RIVET_I_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/ToolHandle.h"
#include "GenInterfaces/IxAODtoHepMCTool.h"

#include "Rivet/AnalysisHandler.hh"

#include "AtlasHepMC/GenEvent.h"

#include <vector>
#include <string>

class ISvcLocator;
class IxAODtoHepMCTool;
//class ITHistSvc;


/// Interface to the Rivet analysis package
/// @author James Monk <jmonk@cern.ch>
/// @author Andy Buckley <andy.buckley@cern.ch>
/// @author Christian Gutschow <chris.g@cern.ch>
class Rivet_i : public AthAlgorithm {
public:

  /// Standard algorithm constructor
  Rivet_i(const std::string &name, ISvcLocator *pSvcLocator);

  /// @brief Initialise the Rivet interface and Athena services.
  ///
  /// Pass a list of analyses to the Rivet AnalysisHandler, which dynamically
  /// loads the necessary libraries (including user-written analyses).
  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;

  /// Run the Rivet analyses on one event, which is retrieved from StoreGate.
  virtual StatusCode execute() override;

  /// Finalise each analysis and commit the plots to an AIDA tree and the
  /// THistSvc ROOT tree.
  virtual StatusCode finalize() override;


private:

  /// Book an AIDA::IDataPointSet into the THistSvc as a TH1D at path @param path
//  StatusCode regHist(const AIDA::IDataPointSet& dps, const std::string& path);

  /// Book an AIDA::IDataPointSet into the THistSvc as a TGraph at path @param path
//  StatusCode regGraph(const AIDA::IDataPointSet& dps, const std::string& path);

  // Check and potentially modify events for correct units, beam particles, ...
  const HepMC::GenEvent* checkEvent(const HepMC::GenEvent& event, const EventContext& ctx);

  // Utility method to convert units of the event
  void MeV2GeV(HepMC::GenEvent* event);

  /// A pointer to the THistSvc
  //ServiceHandle<ITHistSvc> m_histSvc;

  /// The stream name for storing the output plots under (default "/Rivet")
  std::string m_stream;

  /// The base file name to write results to.
  std::string m_file;

  //specify a pre-existing yoda file to initialize from
  std::string m_preload;

  /// @brief The analysis plugin search path
  ///
  /// This will be used to internally set RIVET_ANALYSIS_PATH, if not already set in the environment.
  std::string m_anapath;

  /// @brief Whether to avoid the beam consistency checks
  ///
  /// Default is false: checks will be made to ensure that the supplied
  /// events have beams of the sort that the analysis was written to expect.
  bool m_ignorebeams;

  /// @brief Do we need to convert xAOD::Truth back to HepMC::GenEvemt?
  ///
  /// Default is false: assume user runs on EVNT files
  bool m_needsConversion;

  /// Flag to insert beam protons when they are unavailable in the event
  bool m_patchBeams;

  /// A tool to convert xAOD::Truth to HepMC::GenEvent
  ToolHandle<IxAODtoHepMCTool> m_xAODtoHepMCTool{this, "HepMCTool", "xAODtoHepMCTool"};

  /// @brief Will we convert Rivet's internal histo format into a ROOT histo for streaming with THistSvc?
  ///
  /// The default is yes
  /// Currently (03.01.12) there is no conversion for 2D distributions, in which case you
  /// want to set this to False
  bool m_doRootHistos;

  /// The name of the run (prepended to plot paths).
  std::string m_runname;

  /// The GenEvent StoreGate key (default "GEN_EVENT")
  std::string m_genEventKey;

  /// A Rivet analysis handler
  Rivet::AnalysisHandler* m_analysisHandler;

  /// A list of names of the analyses to run (set from the job properties)
  std::vector<std::string> m_analysisNames;

  /// The cross section for this run of events, set from the job properties.
  double m_crossSection;

  /// The uncertainity of the cross section for this run of events, set from the job properties.
  double m_crossSection_uncert;

  /// Flag to determine whether Rivet init has already happened (in execute())
  bool m_init;

  /// Skip variation weights and only run nominal
  bool m_skipweights;

  /// String of weight names (or regex) to select multiweights
  std::string m_matchWeights;

  /// String of weight names (or regex) to veto multiweights
  std::string m_unmatchWeights;

  /// String to specify non-standard nominal weight
  std::string m_nominalWeightName;

  ///Weight cap to set allowed maximum for weights
  double m_weightcap;

  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey{this
      , "EventInfo"
      , "EventInfo"
      , "ReadHandleKey for xAOD::EventInfo" };
};

#endif
