/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include <TriggerAnalysisAlgorithms/TrigGlobalEfficiencyAlg.h>
#include <xAODEventInfo/EventInfo.h>
#include <SystematicsHandles/SysFilterReporter.h>
#include <SystematicsHandles/SysFilterReporterCombiner.h>
#include "PATCore/PATCoreEnums.h"

CP::TrigGlobalEfficiencyAlg::TrigGlobalEfficiencyAlg(const std::string &name,
						     ISvcLocator *svcLoc)
  : EL::AnaAlgorithm(name, svcLoc)
{
  declareProperty("matchingTool", m_trigMatchingTool, "trigger matching tool");
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

  // prepare trigger data
  TrigGlobEffCorr::ImportData triggerData;
  ANA_CHECK(triggerData.importTriggers());
  const auto & triggerDict = triggerData.getDictionary();
  std::unordered_map<std::string, TrigGlobEffCorr::ImportData::TrigDef> triggerDefs;
  for (const auto &[key, trigDef] : triggerData.getTriggerDefs()) {
    auto it = triggerDict.find(key);
    if (it != triggerDict.end()) {
      triggerDefs[it->second] = trigDef;
    }
  }

  // now we can build a map of triggers per year, and extract the electron/photon legs as needed
  std::unordered_map<std::string, std::vector<std::string> > trigMap;
  trigMap["2015"] = m_trigList_2015;
  trigMap["2016"] = m_trigList_2016;
  trigMap["2017"] = m_trigList_2017;
  trigMap["2018"] = m_trigList_2018;
  trigMap["2022"] = m_trigList_2022;
  trigMap["2023"] = m_trigList_2023;
  trigMap["2024"] = m_trigList_2024;
  trigMap["2025"] = m_trigList_2025;

  // combine all the trigger legs in the expected format
  std::map<std::string, std::string> triggerCombination;
  auto combineStrings = [](const std::vector<std::string>& input ) {
    std::string combinedString;
    bool first = true;
    for (const auto & i : input) {
      if (!first) {
	combinedString += " || ";
      }
      combinedString += i;
      first = false;
    }
    return combinedString;
  };
  for (const auto &[year, triggers] : trigMap) {
    if (triggers.empty()) continue;
    if (triggerCombination.find(year) == triggerCombination.end()) {
      triggerCombination[year] = "";
    } else {
      triggerCombination[year] += " || ";
    }
    triggerCombination[year] += combineStrings(triggers);
  }

  // collect the combined electron and photon trigger keys supported by Egamma
  std::map<std::string,std::string> electronLegsPerKey, photonLegsPerKey;
  if (!m_doMatchingOnly) {
    if (m_isRun3Geo) {
      ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestElectronMapKeys(triggerCombination, "2015_2025/rel22.2/2025_Precision2023_Recommendation", electronLegsPerKey));
    }
    else {
      ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestElectronMapKeys(triggerCombination, "2015_2018/rel21.2/Precision_Summer2020_v1", electronLegsPerKey));
    }
    ANA_CHECK(TrigGlobalEfficiencyCorrectionTool::suggestPhotonMapKeys(triggerCombination, "2015_2018/rel21.2/Summer2020_Rec_v1", photonLegsPerKey));
  }

  std::map<std::string, std::string> legsPerTool;
  auto nameForDefaultSF = ITrigGlobalEfficiencyCorrectionTool::toolnameForDefaultScaleFactor();

  // ELECTRON TOOLS
  ToolHandleArray<IAsgElectronEfficiencyCorrectionTool> electronEffTools, electronSFTools;
  int nTools = 0;
  if (!m_electronsHandle.empty() && !m_doMatchingOnly) {
    if (m_electronID.empty()) ATH_MSG_ERROR("Electron ID was not set for TrigGlobalEfficiencyAlg!");
    for (const auto &[trigKey, triggers] : electronLegsPerKey) {
      if (trigKey == nameForDefaultSF) {  // no tool needed in this case
        auto [itr, added] = legsPerTool.emplace(nameForDefaultSF, triggers);
        if (!added) itr->second += "," + triggers;
        continue;
      }
      nTools++;
      for (bool isSFTool : {true, false}) { // one tool instance for efficiencies, another for scale factors
	auto t = m_electronToolsFactory.emplace(m_electronToolsFactory.end(),
						"AsgElectronEfficiencyCorrectionTool/ElTrigEff_"
						+ std::to_string(isSFTool) + "_" + std::to_string(nTools) + "_" + m_electronID.value() + "_" + m_electronIsol.value());
	if (!m_isRun3Geo) {
	  ANA_CHECK(t->setProperty("MapFilePath", "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"));
	}
	ANA_CHECK(t->setProperty("IdKey", m_electronID.value()));
	ANA_CHECK(t->setProperty("IsoKey", m_electronIsol.value()));
	ANA_CHECK(t->setProperty("TriggerKey", isSFTool ? trigKey : "Eff_" + trigKey ));
	ANA_CHECK(t->setProperty("CorrelationModel", "TOTAL"));
	ANA_CHECK(t->setProperty("ForceDataType", PATCore::ParticleDataType::Full));
	ANA_CHECK(t->setProperty("OutputLevel", msg().level()));
	ANA_CHECK(t->initialize());
	// now record the handle
	auto& handles = (isSFTool? electronSFTools : electronEffTools);
	handles.push_back(t->getHandle());
	std::string name = handles[handles.size()-1].name();
	legsPerTool[name] = triggers;
	// and add the systematics
	ANA_CHECK(m_systematicsList.addSystematics( *handles[handles.size()-1] ));
      }
    }
  }

  // PHOTON TOOLS
  ToolHandleArray<IAsgPhotonEfficiencyCorrectionTool> photonEffTools, photonSFTools;
  nTools = 0;
  if (!m_photonsHandle.empty() && !m_doMatchingOnly) {
    if (m_photonIsol.empty()) ATH_MSG_ERROR("Photon Isolation was not set for TrigGlobalEfficiencyAlg!");
    for (const auto &[trigKey, triggers] : photonLegsPerKey) {
      if (trigKey == nameForDefaultSF) {  // no tool needed in this case
        auto [itr, added] = legsPerTool.emplace(nameForDefaultSF, triggers);
        if (!added) itr->second += "," + triggers;
        continue;
      }
      nTools++;
      for (bool isSFTool : {true, false}) { // one tool instance for efficiencies, another for scale factors
	auto t = m_photonToolsFactory.emplace(m_photonToolsFactory.end(),
					      "AsgPhotonEfficiencyCorrectionTool/PhTrigEff_"
					      + std::to_string(isSFTool) + "_" + std::to_string(nTools) + "_" + m_photonIsol.value());
	if (!m_isRun3Geo) {
	  ANA_CHECK(t->setProperty("MapFilePath", "PhotonEfficiencyCorrection/2015_2018/rel21.2/Summer2020_Rec_v1/map3.txt"));
	}
	ANA_CHECK(t->setProperty("IsoKey", m_photonIsol.value()));
	ANA_CHECK(t->setProperty("TriggerKey", isSFTool ? trigKey : "Eff_" + trigKey ));
	ANA_CHECK(t->setProperty("ForceDataType", PATCore::ParticleDataType::Full));
	ANA_CHECK(t->setProperty("OutputLevel", msg().level()));
	ANA_CHECK(t->initialize());
	// now record the handle
	auto& handles = (isSFTool? photonSFTools : photonEffTools);
	handles.push_back(t->getHandle());
	std::string name = handles[handles.size()-1].name();
	legsPerTool[name] = triggers;
	// and add the systematics
	ANA_CHECK(m_systematicsList.addSystematics( *handles[handles.size()-1] ));
      }
    }
  }

  // MUON TOOLS
  ToolHandleArray<CP::IMuonTriggerScaleFactors> muonTools;
  if (!m_muonsHandle.empty() && !m_doMatchingOnly) {
    if (m_muonID.empty()) ATH_MSG_ERROR("Muon ID was not set for TrigGlobalEfficiencyAlg!");
    m_muonTool = asg::AnaToolHandle<CP::IMuonTriggerScaleFactors>("CP::MuonTriggerScaleFactors/MuonTrigEff_" + m_muonID.value());
    ANA_CHECK(m_muonTool.setProperty("MuonQuality", m_muonID.value()));
    ANA_CHECK(m_muonTool.setProperty("AllowZeroSF", true));
    ANA_CHECK(m_muonTool.initialize());
    // now record the handle
    muonTools.push_back(m_muonTool.getHandle());
    // and add the efficiency systematics
    ANA_CHECK(m_systematicsList.addSystematics( *m_muonTool ));
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

StatusCode CP::TrigGlobalEfficiencyAlg::execute()
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
      ANA_CHECK(m_electronsHandle.retrieve(electrons, syst));
      for (const xAOD::Electron *el: *electrons) {
	if (m_electronSelection.getBool(*el, syst)) selectedElectrons.push_back(el);
      }
    }
    if (!m_photonsHandle.empty()) {
      ANA_CHECK(m_photonsHandle.retrieve(photons, syst));
      for (const xAOD::Photon *ph: *photons) {
	if (m_photonSelection.getBool(*ph, syst)) selectedPhotons.push_back(ph);
      }
    }
    if (!m_muonsHandle.empty()) {
      ANA_CHECK(m_muonsHandle.retrieve(muons, syst));
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
      m_tgecTool->getEfficiencyScaleFactor(selectedElectrons, selectedMuons, selectedPhotons,
					   sf).ignore();
    }

    // check if we have trigger matching
    bool matched = false;
    if (!(selectedElectrons.empty() && selectedMuons.empty() && selectedPhotons.empty())) {
      ANA_CHECK(m_tgecTool->checkTriggerMatching(matched, selectedElectrons, selectedMuons, selectedPhotons));
    }
    if (matched) filter.setPassed(true);

    // decorate them onto the EventInfo
    const xAOD::EventInfo *evtInfo {nullptr};
    ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, syst));
    m_scaleFactorDecoration.set(*evtInfo, sf, syst);
    m_matchingDecoration.set(*evtInfo, matched, syst);

  }

  return StatusCode::SUCCESS;
}

StatusCode CP::TrigGlobalEfficiencyAlg::finalize()
{

  ANA_CHECK (m_filterParams.finalize());
  return StatusCode::SUCCESS;
}
