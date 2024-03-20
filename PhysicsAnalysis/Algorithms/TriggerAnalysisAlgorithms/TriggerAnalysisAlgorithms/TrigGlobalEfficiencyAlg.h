/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#ifndef TRIGGER_ANALYSIS_ALGORITHMS__TRIG_GLOBAL_EFFICIENCY_ALG_H
#define TRIGGER_ANALYSIS_ALGORITHMS__TRIG_GLOBAL_EFFICIENCY_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysFilterReporterParams.h>
#include <AsgTools/PropertyWrapper.h>

// Framework includes
#include "AsgMessaging/AsgMessaging.h"
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <AsgTools/AsgTool.h>
#include <AsgTools/ToolHandle.h>
#include <AsgTools/ToolHandleArray.h>
#include <AsgTools/AnaToolHandle.h>

// Trigger includes
#include <TriggerAnalysisInterfaces/ITrigGlobalEfficiencyCorrectionTool.h>
#include <TriggerMatchingTool/IMatchingTool.h>
#include <TrigGlobalEfficiencyCorrection/ImportData.h>
#include "EgammaAnalysisInterfaces/IAsgElectronEfficiencyCorrectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonEfficiencyCorrectionTool.h"
#include "MuonAnalysisInterfaces/IMuonTriggerScaleFactors.h"

namespace CP
{
  class TrigGlobalEfficiencyAlg : public EL::AnaAlgorithm {
  public:
    TrigGlobalEfficiencyAlg(const std::string& name, ISvcLocator* pSvcLocator = nullptr);

    virtual StatusCode initialize() final override;
    virtual StatusCode execute() final override;
    virtual StatusCode finalize() final override;

  private:
    SysListHandle m_systematicsList {this};

    /// \brief whether to use Run 3 settings
    Gaudi::Property<bool> m_isRun3Geo {this, "isRun3Geo", false, "use Run 3 settings for efficiency correction tools?"};

    /// \brief trigger matching tool
    ToolHandle<Trig::IMatchingTool> m_trigMatchingTool;

    /// \brief Trigger Global Efficiency Correction Tool handle
    asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool> m_tgecTool;

    /// \brief list of triggers or trigger chains
    Gaudi::Property<std::vector<std::string>> m_trigList_2015 {this, "triggers_2015", {}, "2015 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2016 {this, "triggers_2016", {}, "2016 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2017 {this, "triggers_2017", {}, "2017 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2018 {this, "triggers_2018", {}, "2018 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2022 {this, "triggers_2022", {}, "2022 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2023 {this, "triggers_2023", {}, "2023 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2024 {this, "triggers_2024", {}, "2024 trigger selection list"};
    Gaudi::Property<std::vector<std::string>> m_trigList_2025 {this, "triggers_2025", {}, "2025 trigger selection list"};

    /// \brief whether to not apply an event filter
    Gaudi::Property<bool> m_noFilter {this, "noFilter", false, "whether to not apply an event filter"};
    /// \brief the filter reporter params
    SysFilterReporterParams m_filterParams {this, "global trigger matching"};

    /// \brief whether to only run the global trigger matching, and not compute efficiency SFs
    Gaudi::Property<bool> m_doMatchingOnly {this, "doMatchingOnly", false, "whether to disable efficiency SFs and apply matching only"};

    /// \brief decoration of the global trigger SF
    SysWriteDecorHandle<float> m_scaleFactorDecoration {
      this, "scaleFactorDecoration", "", "the decoration for the global trigger efficiency scale factor"
	};

    /// \brief decoration of the global trigger matching flag
    SysWriteDecorHandle<bool> m_matchingDecoration {
      this, "matchingDecoration", "", "the decoration for the global trigger matching decision"
	};

    /// \brief input electron collection
    SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use"
	};

    /// \brief input electron selection
    SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the selection on the input electrons"
	};

    /// \brief input muon collection
    SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use"
	};

    /// \brief input muon selection
    SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the selection on the input muons"
	};

    /// \brief input photon collection
    SysReadHandle<xAOD::PhotonContainer> m_photonsHandle {
      this, "photons", "", "the photon container to use"
	};

    /// \brief input photon selection
    SysReadSelectionHandle m_photonSelection {
      this, "photonSelection", "", "the selection on the input photons"
	};

    /// \brief EventInfo to decorate
    SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfoContainer", "EventInfo", "the EventInfo container to decorate to"
	};


    /// \brief the muon trigger SF handle
    asg::AnaToolHandle<IMuonTriggerScaleFactors> m_muonTool;
    /// \brief RAII on-the-fly tool creation for electrons
    std::vector<asg::AnaToolHandle<IAsgElectronEfficiencyCorrectionTool> > m_electronToolsFactory;
    /// \brief RAII on-the-fly tool creation for photons
    std::vector<asg::AnaToolHandle<IAsgPhotonEfficiencyCorrectionTool> > m_photonToolsFactory;

    /// \brief electron ID
    Gaudi::Property<std::string> m_electronID {this, "electronID", "", "electron ID WP"};
    /// \brief electron Isolation
    Gaudi::Property<std::string> m_electronIsol {this, "electronIsol", "", "electron Isolation WP"};
    /// \brief photon Isolation
    Gaudi::Property<std::string> m_photonIsol {this, "photonIsol", "", "photon Isolation WP"};
    /// \brief muon quality
    Gaudi::Property<std::string> m_muonID {this, "muonID", "", "muon ID/Quality WP"};

  }; // class TrigGlobalEfficiencyAlg
} // namespace CP

#endif /* TRIGGER_ANALYSIS_ALGORITHMS__TRIG_GLOBAL_EFFICIENCY_ALG_H */
