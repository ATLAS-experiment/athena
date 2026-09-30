/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include <TriggerAnalysisAlgorithms/TrigGlobalEfficiencyAlg.h>
#include <TriggerAnalysisAlgorithms/TrigChainNameHelpers.h>
#include <xAODEventInfo/EventInfo.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include <TrigGlobalEfficiencyCorrection/TrigGlobalEfficiencyCorrectionTool.h>
#include <AsgTools/AsgToolConfig.h>
#include "PATCore/PATCoreEnums.h"

#include <utility>

CP::TrigGlobalEfficiencyAlg::TrigGlobalEfficiencyAlg(const std::string &name,
						     ISvcLocator *svcLoc)
  : EL::AnaAlgorithm(name, svcLoc)
{
  declareProperty("matchingTool", m_trigMatchingTool, "trigger matching tool");
}

template <typename ToolInterface, typename SetToolPropertiesFn>
StatusCode CP::TrigGlobalEfficiencyAlg::makeEgammaTools(
    std::vector<ToolHandle<ToolInterface> >& toolsFactory,
    ToolHandleArray<ToolInterface>& effTools,
    ToolHandleArray<ToolInterface>& sfTools,
    const std::map<std::string, std::string>& legsPerKey,
    const std::string& toolNamePrefix,
    const std::string& toolNameSuffix,
    const SetToolPropertiesFn& setToolProperties,
    std::map<std::string, std::string>& legsPerTool)
{
  const auto nameForDefaultSF = ITrigGlobalEfficiencyCorrectionTool::toolnameForDefaultScaleFactor();
  int nTools = 0;
  for (const auto &[trigKey, triggers] : legsPerKey) {
    if (trigKey == nameForDefaultSF) {  // no tool needed in this case
      auto [itr, added] = legsPerTool.emplace(nameForDefaultSF, triggers);
      if (!added) itr->second += "," + triggers;
      continue;
    }
    nTools++;
    for (bool isSFTool : {true, false}) { // one tool instance for efficiencies, another for scale factors
      asg::AsgToolConfig config(toolNamePrefix + std::to_string(isSFTool) + "_" + std::to_string(nTools) + toolNameSuffix);
      ANA_CHECK(setToolProperties(config));
      ANA_CHECK(config.setProperty("TriggerKey", isSFTool ? trigKey : "Eff_" + trigKey ));
      ANA_CHECK(config.setProperty("ForceDataType", PATCore::ParticleDataType::Full));
      ANA_CHECK(config.setProperty("OutputLevel", msg().level()));
      ToolHandle<ToolInterface>& t = toolsFactory.emplace_back();
      std::shared_ptr<void> cleanup;
      ANA_CHECK(config.makeTool(t, cleanup));
      m_toolsCleanup.push_back(std::move(cleanup));
      // now record the handle
      auto& handles = (isSFTool? sfTools : effTools);
      handles.push_back(t);
      legsPerTool[handles[handles.size()-1].name()] = triggers;
      // and add the systematics
      ANA_CHECK(m_systematicsList.addSystematics( *handles[handles.size()-1] ));
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode CP::TrigGlobalEfficiencyAlg::initialize()
{

  if (m_trigList_2015.empty() && m_trigList_2016.empty() && m_trigList_2017.empty() && m_trigList_2018.empty()
      && m_trigList_2022.empty() && m_trigList_2023.empty() && m_trigList_2024.empty() && m_trigList_2025.empty()) {
    ATH_MSG_ERROR("A list of triggers needs to be provided");
    return StatusCode::FAILURE;
  }

  ANA_CHECK(m_electronsHandle.initialize(m_systematicsList, SG::AllowEmpty));
  ANA_CHECK(m_muonsHandle.initialize(m_systematicsList, SG::AllowEmpty));
  ANA_CHECK(m_photonsHandle.initialize(m_systematicsList, SG::AllowEmpty));

  ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle, SG::AllowEmpty));
  ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle, SG::AllowEmpty));
  ANA_CHECK(m_photonSelection.initialize(m_systematicsList, m_photonsHandle, SG::AllowEmpty));

  ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
  ANA_CHECK(m_scaleFactorDecoration.initialize(m_systematicsList, m_eventInfoHandle));
  ANA_CHECK(m_matchingDecoration.initialize(m_systematicsList, m_eventInfoHandle));

  ANA_CHECK (m_filterParams.initialize(m_systematicsList));

  // retrieve the trigger matching tool
  ANA_CHECK(m_trigMatchingTool.retrieve());

  // now we can build a map of triggers per year, and extract the electron/photon legs as needed
  const std::vector<std::pair<std::string, const std::vector<std::string>*> > trigMap {
    {"2015", &m_trigList_2015.value()},
    {"2016", &m_trigList_2016.value()},
    {"2017", &m_trigList_2017.value()},
    {"2018", &m_trigList_2018.value()},
    {"2022", &m_trigList_2022.value()},
    {"2023", &m_trigList_2023.value()},
    {"2024", &m_trigList_2024.value()},
    {"2025", &m_trigList_2025.value()}};

  // combine all the trigger legs in the expected format
  std::map<std::string, std::string> triggerCombination;
  for (const auto &[year, triggers] : trigMap) {
    if (triggers->empty()) continue;
    std::string& combinedString = triggerCombination[year];
    combinedString = triggers->front();
    for (std::size_t i = 1; i < triggers->size(); ++i) {
      combinedString += " || " + (*triggers)[i];
    }
  }

  // collect the combined electron and photon trigger keys supported by Egamma
  std::map<std::string,std::string> electronLegsPerKey, photonLegsPerKey;
  if (!m_doMatchingOnly) {
    if (m_isRun3Geo) {
      ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestElectronMapKeys(triggerCombination, "2015_2025/rel22.2/2025_Run3_Consolidated_Recommendation_v4", electronLegsPerKey));
    }
    else {
      ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestElectronMapKeys(triggerCombination, "2015_2018/rel21.2/Precision_Summer2020_v1", electronLegsPerKey));
    }
    ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestPhotonMapKeys(triggerCombination, "2015_2018/rel21.2/Summer2020_Rec_v1", photonLegsPerKey));
  }

  std::map<std::string, std::string> legsPerTool;

  // ELECTRON TOOLS
  ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> electronEffTools, electronSFTools;
  if (!m_electronsHandle.empty() && !m_doMatchingOnly) {
    if (m_electronID.empty()) {
      ATH_MSG_ERROR("Electron ID was not set for TrigGlobalEfficiencyAlg!");
      return StatusCode::FAILURE;
    }
    ANA_CHECK(makeEgammaTools(m_electronToolsFactory, electronEffTools, electronSFTools, electronLegsPerKey,
                              "AsgElectronEfficiencyCorrectionTool/ElTrigEff_",
                              "_" + m_electronID.value() + "_" + m_electronIsol.value(),
                              [this](asg::AsgToolConfig& config) -> StatusCode {
                                ANA_CHECK(config.setProperty("MapFilePath", std::string("ElectronEfficiencyCorrection/2015_2025/rel22.2/") +
                                                        (m_isRun3Geo ? "2025_Run3_Consolidated_Recommendation_v4/map2.txt" : "2025_Run2Rel22_Recommendation_v3/map1.txt")));
                                ANA_CHECK(config.setProperty("IdKey", m_electronID.value()));
                                ANA_CHECK(config.setProperty("IsoKey", m_electronIsol.value()));
                                ANA_CHECK(config.setProperty("CorrelationModel", "TOTAL"));
                                return StatusCode::SUCCESS;
                              },
                              legsPerTool));
  }

  // PHOTON TOOLS
  ToolHandleArray<IAsgPhotonEfficiencyCorrectionTool> photonEffTools, photonSFTools;
  if (!m_photonsHandle.empty() && !m_doMatchingOnly) {
    if (m_photonIsol.empty()) {
      ATH_MSG_ERROR("Photon Isolation was not set for TrigGlobalEfficiencyAlg!");
      return StatusCode::FAILURE;
    }
    ANA_CHECK(makeEgammaTools(m_photonToolsFactory, photonEffTools, photonSFTools, photonLegsPerKey,
                              "AsgPhotonEfficiencyCorrectionTool/PhTrigEff_",
                              "_" + m_photonIsol.value(),
                              [this](asg::AsgToolConfig& config) -> StatusCode {
                                if (!m_isRun3Geo) {
                                  ANA_CHECK(config.setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2018/rel21.2/Summer2020_Rec_v1/map3.txt"));
                                }
                                ANA_CHECK(config.setProperty("IsoKey", m_photonIsol.value()));
                                return StatusCode::SUCCESS;
                              },
                              legsPerTool));
  }

  // MUON TOOLS
  ToolHandleArray<CP::IMuonTriggerScaleFactors> muonTools;
  if (!m_muonsHandle.empty() && !m_doMatchingOnly) {
    if (m_muonID.empty()) {
      ATH_MSG_ERROR("Muon ID was not set for TrigGlobalEfficiencyAlg!");
      return StatusCode::FAILURE;
    }
    m_muonTool = asg::AnaToolHandle<CP::IMuonTriggerScaleFactors>("CP::MuonTriggerScaleFactors/MuonTrigEff_" + m_muonID.value());
    ANA_CHECK(m_muonTool.setProperty("MuonQuality", m_muonID.value()));
    ANA_CHECK(m_muonTool.setProperty("AllowZeroSF", true));
    ANA_CHECK(m_muonTool.setProperty("Campaign", m_campaign.value()));
    ANA_CHECK(m_muonTool.initialize());
    // now record the handle
    muonTools.push_back(m_muonTool.getHandle());
    // and add the efficiency systematics
    ANA_CHECK(m_systematicsList.addSystematics( *m_muonTool ));
  }

  // create the individual trigger-matching decorators
  for(const std::string& trig : m_separateMatchingTriggers) {
    m_separateMatchingDecorators.emplace(trig, SysWriteDecorHandle<bool>("triggerMatch_"+sanitizeTriggerChainName(trig)+m_separateMatchingDecorSuffix+"_%SYS%", this));
    ANA_CHECK(m_separateMatchingDecorators.at(trig).initialize(m_systematicsList, m_eventInfoHandle));

    m_separateMatchingFlags[trig] = false;
  }

  // finally, set up the global trigger tool
  m_tgecTool = asg::AnaToolHandle<ITrigGlobalEfficiencyCorrectionTool>("TrigGlobalEfficiencyCorrectionTool/TrigGlobal_" + this->name() );
  ANA_CHECK(m_tgecTool.setProperty("ElectronEfficiencyTools", electronEffTools));
  ANA_CHECK(m_tgecTool.setProperty("ElectronScaleFactorTools", electronSFTools));
  ANA_CHECK(m_tgecTool.setProperty("PhotonEfficiencyTools", photonEffTools));
  ANA_CHECK(m_tgecTool.setProperty("PhotonScaleFactorTools", photonSFTools));
  ANA_CHECK(m_tgecTool.setProperty("MuonTools", muonTools));
  ANA_CHECK(m_tgecTool.setProperty("ListOfLegsPerTool", legsPerTool));
  ANA_CHECK(m_tgecTool.setProperty("TriggerCombination", triggerCombination));
  ANA_CHECK(m_tgecTool.setProperty("TriggerMatchingTool", m_trigMatchingTool));
  ANA_CHECK(m_tgecTool.setProperty("NumberOfToys", m_numToys));
  ANA_CHECK(m_tgecTool.setProperty("OutputLevel", MSG::ERROR));
  ANA_CHECK(m_tgecTool.initialize());

  ANA_CHECK(m_systematicsList.initialize());

  return StatusCode::SUCCESS;
}

StatusCode CP::TrigGlobalEfficiencyAlg::execute(const EventContext& ctx)
{

  CP::SysFilterReporterCombiner filterCombiner (m_filterParams, m_noFilter.value());

  for (const auto & syst : m_systematicsList.systematicsVector()) {
    CP::SysFilterReporter filter (filterCombiner, syst);

    // retrieve lepton collections with selections
    const xAOD::ElectronContainer* electrons(nullptr);
    const xAOD::PhotonContainer* photons(nullptr);
    const xAOD::MuonContainer* muons(nullptr);
    std::vector<const xAOD::Electron*> selectedElectrons;
    std::vector<const xAOD::Photon*> selectedPhotons;
    std::vector<const xAOD::Muon*> selectedMuons;

    if (!m_electronsHandle.empty()) {
      ANA_CHECK(m_electronsHandle.retrieve(electrons, syst, ctx));
      for (const xAOD::Electron *el: *electrons) {
        if (m_electronSelection.getBool(*el, syst)) selectedElectrons.push_back(el);
      }
    }
    if (!m_photonsHandle.empty()) {
      ANA_CHECK(m_photonsHandle.retrieve(photons, syst, ctx));
      for (const xAOD::Photon *ph: *photons) {
        if (m_photonSelection.getBool(*ph, syst)) selectedPhotons.push_back(ph);
      }
    }
    if (!m_muonsHandle.empty()) {
      ANA_CHECK(m_muonsHandle.retrieve(muons, syst, ctx));
      for (const xAOD::Muon *mu: *muons) {
        if (m_muonSelection.getBool(*mu, syst)) selectedMuons.push_back(mu);
      }
    }

    ANA_CHECK(m_tgecTool->applySystematicVariation(syst));

    // compute the scale factor
    double sf;
    if (selectedElectrons.empty() && selectedMuons.empty() && selectedPhotons.empty()) sf = 1.0;
    else if (m_doMatchingOnly) sf = 1.0;
    else {
      sf = NAN;
      const CP::CorrectionCode code = m_tgecTool->getEfficiencyScaleFactor(selectedElectrons, selectedMuons, selectedPhotons, sf);
      if (code != CP::CorrectionCode::Ok) {
        ++m_nScaleFactorFailures;
        if (m_nScaleFactorFailures <= 10) {
          ATH_MSG_WARNING("Global trigger scale factor computation failed (CorrectionCode " << code.code() << ") for systematic '" << syst.name() << "', scale factor set to " << sf
                          << (m_nScaleFactorFailures == 10 ? " (further warnings suppressed)" : ""));
        }
      }
    }

    // Retrieve EventInfo
    const xAOD::EventInfo *evtInfo {nullptr};
    ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, syst, ctx));

    // Check if we have trigger matching
    bool matched = false;
    if (!(selectedElectrons.empty() && selectedMuons.empty() && selectedPhotons.empty())) {
      if(m_separateMatchingTriggers.value().empty()) {
        ANA_CHECK(m_tgecTool->checkTriggerMatching(matched, selectedElectrons, selectedMuons, selectedPhotons));
      } else {
        ANA_CHECK(m_tgecTool->checkTriggerMatching(m_separateMatchingFlags, selectedElectrons, selectedMuons, selectedPhotons));
        for(const auto& [trig, flag] : m_separateMatchingFlags) {
          if(flag) matched = true;
          m_separateMatchingDecorators.at(trig).set(*evtInfo, flag, syst);
        }
      }
    } else {
      // no selected leptons: still write the per-trigger matching decorations
      for (const auto& [trig, decorator] : m_separateMatchingDecorators) {
        decorator.set(*evtInfo, false, syst);
      }
    }
    if (matched) filter.setPassed(true);

    // Decorate global outputs onto the EventInfo
    m_scaleFactorDecoration.set(*evtInfo, sf, syst);
    m_matchingDecoration.set(*evtInfo, matched, syst);
  }

  return StatusCode::SUCCESS;
}

StatusCode CP::TrigGlobalEfficiencyAlg::finalize()
{
  if (m_nScaleFactorFailures > 0) {
    ATH_MSG_WARNING("Global trigger scale factor computation failed " << m_nScaleFactorFailures << " times");
  }

  ANA_CHECK (m_filterParams.finalize());
  return StatusCode::SUCCESS;
}
