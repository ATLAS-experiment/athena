/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#include "KLFitterAnalysisAlgorithms/RunKLFitterAlg.h"

#include "AthContainers/ConstDataVector.h"
#include "PathResolver/PathResolver.h"

namespace EventReco {

StatusCode RunKLFitterAlg::initialize() {
  ANA_CHECK(m_electronsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_muonsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_jetsHandle.initialize(m_systematicsList));
  ANA_CHECK(m_metHandle.initialize(m_systematicsList));
  ANA_CHECK(m_eventInfoHandle.initialize(m_systematicsList));
  ANA_CHECK(m_electronSelection.initialize(m_systematicsList, m_electronsHandle,
                                           SG::AllowEmpty));
  ANA_CHECK(m_muonSelection.initialize(m_systematicsList, m_muonsHandle,
                                       SG::AllowEmpty));
  ANA_CHECK(m_jetSelection.initialize(m_systematicsList, m_jetsHandle,
                                      SG::AllowEmpty));

  ANA_CHECK(m_selection.initialize(m_systematicsList, m_eventInfoHandle));

  ANA_CHECK(m_outHandle.initialize(m_systematicsList));

  ANA_CHECK(m_systematicsList.initialize());

  // parse likelihood type
  try {
    m_LHTypeEnum = KLFEnums::strToLikelihood.at(m_LHType.value());
  } catch (std::out_of_range &) {
    ANA_MSG_ERROR("Unrecognized KLFitter likelihood: "
                  << m_LHType.value() << ". Available options: "
                  << KLFEnums::printEnumOptions(KLFEnums::strToLikelihood));
    return StatusCode::FAILURE;
  }

  if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_JetAngles) {
    ANA_MSG_ERROR("The ttbar_JetAngles likelihood is currently not supported!");
    return StatusCode::FAILURE;
  }

  // parse lepton type
  try {
    m_leptonTypeEnum = KLFEnums::strToLeptonType.at(m_leptonType.value());
    if (m_leptonTypeEnum != KLFEnums::LeptonType::kNoLepton &&
        m_LHTypeEnum == KLFEnums::Likelihood::ttbar_AllHad) {
      ANA_MSG_ERROR(
          "If using ttbar_AllHad likelihood, please use leptonType = "
          "kNoLepton.");
      return StatusCode::FAILURE;
    }
  } catch (std::out_of_range &) {
    ANA_MSG_ERROR("Unrecognized KLFitter leptonType: "
                  << m_leptonType.value() << ". Available options: "
                  << KLFEnums::printEnumOptions(KLFEnums::strToLeptonType));
    return StatusCode::FAILURE;
  }

  // parse jet selection
  try {
    m_jetSelectionModeEnum =
        KLFEnums::strToJetSelection.at(m_jetSelectionMode.value());
  } catch (std::out_of_range &) {
    ANA_MSG_ERROR("Unrecognized KLFitter JetSelectionMode: "
                  << m_jetSelectionMode.value() << ". Available options: "
                  << KLFEnums::printEnumOptions(KLFEnums::strToJetSelection));
    return StatusCode::FAILURE;
  }

  if (m_jetSelectionModeEnum > KLFEnums::JetSelectionMode::kLeadingEight)
    m_useBtagPriority = true;
  try {
    m_njetsRequirement = KLFEnums::jetSelToNumber.at(m_jetSelectionModeEnum);
  } catch (std::out_of_range &) {
    ANA_MSG_ERROR(
        "Could not parse the number of required jets from KLFitter jet "
        "selection mode: "
        << m_jetSelectionMode.value());
    return StatusCode::FAILURE;
  }

  // parse b-tagging method
  try {
    m_bTaggingMethodEnum =
        KLFEnums::strToBtagMethod.at(m_bTaggingMethod.value());
  } catch (std::out_of_range &) {
    ANA_MSG_ERROR("Unrecognized KLFitter BTaggingMethod: "
                  << m_bTaggingMethod.value() << ". Available options: "
                  << KLFEnums::printEnumOptions(KLFEnums::strToBtagMethod));
    return StatusCode::FAILURE;
  }

  // setup the KLFitter::Fitter instance
  m_myFitter = std::make_unique<KLFitter::Fitter>();
  const std::string transferFunctionAbsPath =
      PathResolverFindCalibDirectory(m_transferFunctionsPath.value());
  m_myDetector =
      std::make_unique<KLFitter::DetectorAtlas_8TeV>(transferFunctionAbsPath);
  if (!m_myFitter->SetDetector(m_myDetector.get())) {
    ANA_MSG_ERROR(
        "Failed to set KLFitter::Detector for KLFitter::Fitter instance.");
    return StatusCode::FAILURE;
  }

  // create the likelihoods
  m_myLikelihood = std::make_unique<KLFitter::LikelihoodTopLeptonJets>();
  m_myLikelihood_TTH = std::make_unique<KLFitter::LikelihoodTTHLeptonJets>();
  m_myLikelihood_JetAngles =
      std::make_unique<KLFitter::LikelihoodTopLeptonJets_JetAngles>();
  m_myLikelihood_Angular =
      std::make_unique<KLFitter::LikelihoodTopLeptonJets_Angular>();
  m_myLikelihood_TTZ = std::make_unique<KLFitter::LikelihoodTTZTrilepton>();
  m_myLikelihood_AllHadronic =
      std::make_unique<KLFitter::LikelihoodTopAllHadronic>();
  m_myLikelihood_BoostedLJets =
      std::make_unique<KLFitter::BoostedLikelihoodTopLeptonJets>();

  // SetleptonType
  if (m_LHTypeEnum != KLFEnums::Likelihood::ttbar_AllHad) {
    if (m_leptonTypeEnum == KLFEnums::LeptonType::kElectron) {
      m_leptonTypeKLFitterEnum =
          KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_TTH =
          KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_JetAngles =
          KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_Angular =
          KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_TTZ =
          KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_BoostedLJets =
          KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron;
    } else if (m_leptonTypeEnum == KLFEnums::LeptonType::kMuon) {
      m_leptonTypeKLFitterEnum =
          KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_TTH =
          KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_JetAngles =
          KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_Angular =
          KLFitter::LikelihoodTopLeptonJets_Angular::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_TTZ =
          KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_BoostedLJets =
          KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon;
    } else if (m_leptonTypeEnum == KLFEnums::LeptonType::kTriElectron) {
      if (m_LHType.value() != KLFEnums::Likelihood::ttZTrilepton) {
        ANA_MSG_ERROR(
            " LeptonType kTriElectron is only defined for the ttZTrilepton "
            "likelihood");
        return StatusCode::FAILURE;
      }
      m_leptonTypeKLFitterEnum =
          KLFitter::LikelihoodTopLeptonJets::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_TTH =
          KLFitter::LikelihoodTTHLeptonJets::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_JetAngles =
          KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_TTZ =
          KLFitter::LikelihoodTTZTrilepton::LeptonType::kElectron;
      m_leptonTypeKLFitterEnum_BoostedLJets =
          KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kElectron;
    } else if (m_leptonTypeEnum == KLFEnums::LeptonType::kTriMuon) {
      if (m_LHType.value() != KLFEnums::Likelihood::ttZTrilepton) {
        ANA_MSG_ERROR(
            " LeptonType kTriMuon is only defined for the ttZTrilepton "
            "likelihood");
        return StatusCode::FAILURE;
      }
      m_leptonTypeKLFitterEnum =
          KLFitter::LikelihoodTopLeptonJets::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_TTH =
          KLFitter::LikelihoodTTHLeptonJets::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_JetAngles =
          KLFitter::LikelihoodTopLeptonJets_JetAngles::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_TTZ =
          KLFitter::LikelihoodTTZTrilepton::LeptonType::kMuon;
      m_leptonTypeKLFitterEnum_BoostedLJets =
          KLFitter::BoostedLikelihoodTopLeptonJets::LeptonType::kMuon;
    } else {
      ANA_MSG_ERROR(" Please supply a valid LeptonType : kElectron or kMuon");
      return StatusCode::FAILURE;
    }

    m_myLikelihood->SetLeptonType(m_leptonTypeKLFitterEnum);
    m_myLikelihood_TTH->SetLeptonType(m_leptonTypeKLFitterEnum_TTH);
    m_myLikelihood_JetAngles->SetLeptonType(m_leptonTypeKLFitterEnum_JetAngles);
    m_myLikelihood_Angular->SetLeptonType(m_leptonTypeKLFitterEnum_Angular);
    m_myLikelihood_TTZ->SetLeptonType(m_leptonTypeKLFitterEnum_TTZ);
    m_myLikelihood_BoostedLJets->SetLeptonType(
        m_leptonTypeKLFitterEnum_BoostedLJets);
  }

  m_myLikelihood->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_TTH->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_JetAngles->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_Angular->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_TTZ->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_AllHadronic->SetBTagging(m_bTaggingMethodEnum);
  m_myLikelihood_BoostedLJets->SetBTagging(m_bTaggingMethodEnum);
  // set top mass
  m_myLikelihood->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_TTH->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_JetAngles->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_Angular->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_TTZ->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_AllHadronic->PhysicsConstants()->SetMassTop(m_massTop);
  m_myLikelihood_BoostedLJets->PhysicsConstants()->SetMassTop(m_massTop);

  // whether the top mass is fixed to the constant in likelihood or not
  m_myLikelihood->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_TTH->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_JetAngles->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_Angular->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_TTZ->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_AllHadronic->SetFlagTopMassFixed(m_fixedTopMass);
  m_myLikelihood_BoostedLJets->SetFlagTopMassFixed(m_fixedTopMass);

  // configure which likelihood to use in the fitter
  int klfitter_returncode = 0;
  if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar) {
    klfitter_returncode = m_myFitter->SetLikelihood(m_myLikelihood.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttH) {
    klfitter_returncode = m_myFitter->SetLikelihood(m_myLikelihood_TTH.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_JetAngles) {
    klfitter_returncode =
        m_myFitter->SetLikelihood(m_myLikelihood_JetAngles.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_Angular) {
    klfitter_returncode =
        m_myFitter->SetLikelihood(m_myLikelihood_Angular.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttZTrilepton &&
             (m_leptonTypeEnum == KLFEnums::LeptonType::kTriElectron ||
              m_leptonTypeEnum == KLFEnums::LeptonType::kTriMuon)) {
    // For ttZ->trilepton, we can have difficult combinations of leptons in the
    // final state (3x same flavour, or mixed case). The latter is trivial, for
    // which we can default back to the ljets likelihood. So we distinguish
    // here:
    //  - kTriMuon, kTriElectron: dedicated TTZ->trilepton likelihood,
    //  - kMuon, kElectron: standard ttbar->l+jets likelihood.
    klfitter_returncode = m_myFitter->SetLikelihood(m_myLikelihood_TTZ.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttZTrilepton) {
    klfitter_returncode = m_myFitter->SetLikelihood(m_myLikelihood.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_AllHad) {
    klfitter_returncode =
        m_myFitter->SetLikelihood(m_myLikelihood_AllHadronic.get());
  } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_BoostedLJets) {
    klfitter_returncode =
        m_myFitter->SetLikelihood(m_myLikelihood_BoostedLJets.get());
  } else {
    ANA_MSG_ERROR("Unrecognized KLFitter likelihood: " << m_LHType.value());
    return StatusCode::FAILURE;
  }

  if (!klfitter_returncode) {
    ANA_MSG_ERROR("Failed to SetLikelihood for likelihood "
                  << m_LHType.value());
    return StatusCode::FAILURE;
  }

  if (m_bTagDecoration.value().find("Continuous") != std::string::npos) {
    ANA_MSG_ERROR("KLFitter cannot run using Continuous b-tag working point!");
    return StatusCode::FAILURE;
  }
  m_bTagDecoAcc = std::make_unique<SG::AuxElement::ConstAccessor<char>>(
      m_bTagDecoration.value());

  if (m_bTaggingMethodEnum ==
      KLFitter::LikelihoodBase::BtaggingMethod::kWorkingPoint) {
    ANA_CHECK(m_btagging_eff_tool.retrieve());
  }

  ANA_MSG_INFO("++++++++++++++++++++++++++++++");
  ANA_MSG_INFO("Configured KLFitter with name " << name());
  ANA_MSG_INFO("  Using " << m_btagging_eff_tool);
  ANA_MSG_INFO("  Using transfer functions with full path "
               << transferFunctionAbsPath);
  ANA_MSG_INFO("  Using Lepton \t\t" << m_leptonType.value());
  ANA_MSG_INFO("  Using JetSelectionMode \t" << m_jetSelectionMode.value());
  ANA_MSG_INFO("  Using BTaggingMethod \t" << m_bTaggingMethod.value());
  ANA_MSG_INFO("  Using TopMassFixed \t" << m_fixedTopMass);

  if (m_saveAllPermutations)
    ANA_MSG_INFO("  Saving All permutations");
  else
    ANA_MSG_INFO(
        "  Saving only the permutation with the highest event probability");
  ANA_MSG_INFO("++++++++++++++++++++++++++++++");

  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::execute() {
  for (const auto &sys : m_systematicsList.systematicsVector()) {
    ANA_CHECK(execute_syst(sys));
  }
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::execute_syst(const CP::SystematicSet &sys) {
  // run KLFitter
  // create an instance of the particles class filled with the particles to be
  // fitted; here, you need to make sure that
  // - the particles are in the range allowed by the transfer functions (eta and
  // pt)
  // - the energies and momenta are in GeV
  // - be aware that *all* particles you're adding are considered in the fit
  //   (many particles lead to many permutations to be considered and hence a
  //   long running time and not necessarily good fitting results due to the
  //   many available permutations)
  // the arguments taken py AddParticle() are
  // - TLorentzVector of the physics 4-momentum
  // - detector eta for the evaluation of the transfer functions (for muons:
  // just use the physics eta)
  // - type of particle
  // - an optional name of the particle (pass empty string in case you don't
  // want to give your particle a name)
  // - index of the particle in your original collection (for convenience)
  // - for jets:
  //   * bool isBtagged : mandatory only if you want to use b-tagging in the fit

  // first figure out if this event even passes the selection in which we are to
  // run this KLFitter instance
  const xAOD::EventInfo *evtInfo = nullptr;
  ANA_CHECK(m_eventInfoHandle.retrieve(evtInfo, sys));

  if (!m_selection.getBool(*evtInfo, sys))
    return StatusCode::SUCCESS;

  const xAOD::ElectronContainer *electrons = nullptr;
  ANA_CHECK(m_electronsHandle.retrieve(electrons, sys));
  const xAOD::MuonContainer *muons = nullptr;
  ANA_CHECK(m_muonsHandle.retrieve(muons, sys));
  const xAOD::JetContainer *jets = nullptr;
  ANA_CHECK(m_jetsHandle.retrieve(jets, sys));
  const xAOD::MissingETContainer *met = nullptr;
  ANA_CHECK(m_metHandle.retrieve(met, sys));

  // perform selection of objects
  KLFitter::Particles *myParticles = new KLFitter::Particles{};

  std::vector<const xAOD::Electron *> selected_electrons;
  std::vector<const xAOD::Muon *> selected_muons;
  std::vector<const xAOD::Jet *> selected_jets;

  // select particles
  for (const xAOD::Electron *el : *electrons) {
    if (m_electronSelection.getBool(*el, sys))
      selected_electrons.push_back(el);
  }

  for (const xAOD::Muon *mu : *muons) {
    if (m_muonSelection.getBool(*mu, sys))
      selected_muons.push_back(mu);
  }

  for (const xAOD::Jet *jet : *jets) {
    if (m_jetSelection.getBool(*jet, sys))
      selected_jets.push_back(jet);
  }

  std::vector<size_t> electron_indices;
  const std::vector<const xAOD::Electron *> selected_sorted_electrons =
      sortPt(selected_electrons, electron_indices);
  std::vector<size_t> muon_indices;
  const std::vector<const xAOD::Muon *> selected_sorted_muons =
      sortPt(selected_muons, muon_indices);
  std::vector<size_t> jet_indices;
  const std::vector<const xAOD::Jet *> selected_sorted_jets =
      sortPt(selected_jets, jet_indices);

  // add leptons to KLFitter particles (not for ttbar all hadronic)
  if (m_LHTypeEnum != KLFEnums::Likelihood::ttbar_AllHad)
    ANA_CHECK(add_leptons(selected_sorted_electrons, selected_sorted_muons,
                          myParticles));

  // add jets to KLFitter particles
  ANA_CHECK(add_jets(selected_sorted_jets, myParticles));

  // add the particles to the fitter itself
  if (!m_myFitter->SetParticles(myParticles)) {
    ANA_MSG_ERROR("Error adding particles to KLFitter");
    return StatusCode::FAILURE;
  }

  // add MET
  auto *met_finalTrk = (*met)[m_METterm.value()];
  if (!met_finalTrk) {
    ANA_MSG_ERROR("RunKLFitterAlg: Error retrieving MET term "
                  << m_METterm.value());
    return StatusCode::FAILURE;
  }
  if (!m_myFitter->SetET_miss_XY_SumET(met_finalTrk->mpx() / 1.e3,
                                       met_finalTrk->mpy() / 1.e3,
                                       met_finalTrk->sumet())) {
    ANA_MSG_ERROR("Error adding MET term to KLFitter");
    return StatusCode::FAILURE;
  }

  ANA_CHECK(
      evaluatePermutations(sys, electron_indices, muon_indices, jet_indices));

  delete myParticles;

  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::add_leptons(
    const std::vector<const xAOD::Electron *> &selected_electrons,
    const std::vector<const xAOD::Muon *> &selected_muons,
    KLFitter::Particles *myParticles) {
  // likelihoods with single lepton (either l+jets or ttZ 3lepton mixed lepton
  // flavour)
  if (m_leptonTypeEnum == KLFEnums::LeptonType::kElectron) {
    // for the lep+jets channel, we assume that your leading-pT lepton is the
    // only selected lepton
    TLorentzVector el;
    if (selected_electrons.size() == 0) {
      ANA_MSG_ERROR(
          "For single-lepton kElectron KLFitter likelihoods, at least one "
          "electron is required");
      return StatusCode::FAILURE;
    }
    const xAOD::Electron *xaod_el = selected_electrons.at(0);
    el.SetPtEtaPhiE(xaod_el->pt() / 1.e3, xaod_el->eta(), xaod_el->phi(),
                    xaod_el->e() / 1.e3);
    myParticles->AddParticle(&el, xaod_el->caloCluster()->etaBE(2),
                             KLFitter::Particles::kElectron);
  } else if (m_leptonTypeEnum == KLFEnums::LeptonType::kMuon) {
    TLorentzVector mu;
    if (selected_muons.size() == 0) {
      ANA_MSG_ERROR(
          "For single-lepton kMuon KLFitter likelihoods, at least one muon is "
          "required");
      return StatusCode::FAILURE;
    }
    const xAOD::Muon *xaod_mu = selected_muons.at(0);
    mu.SetPtEtaPhiE(xaod_mu->pt() / 1.e3, xaod_mu->eta(), xaod_mu->phi(),
                    xaod_mu->e() / 1.e3);
    myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon);
  } else if (m_leptonTypeEnum ==
             KLFEnums::LeptonType::kTriElectron) {  // ttZ trilep
    if (selected_electrons.size() < 3) {
      ANA_MSG_ERROR(
          "For tri-lepton kTriElectron KLFitter likelihoods, at least 3 "
          "electrons are required");
      return StatusCode::FAILURE;
    }
    TLorentzVector el;
    for (size_t i = 0; i < 3; ++i) {
      const xAOD::Electron *electron = selected_electrons.at(i);
      el.SetPtEtaPhiE(electron->pt() / 1.e3, electron->eta(), electron->phi(),
                      electron->e() / 1.e3);
      myParticles->AddParticle(&el, electron->caloCluster()->etaBE(2),
                               KLFitter::Particles::kElectron, "", i);
    }
  } else if (m_leptonTypeEnum ==
             KLFEnums::LeptonType::kTriMuon) {  // ttZ trilep
    if (selected_muons.size() < 3) {
      ANA_MSG_ERROR(
          "For tr-lepton kTriMuons KLFitter likelihoods, at least 3 muons are "
          "required");
      return StatusCode::FAILURE;
    }
    TLorentzVector mu;
    for (size_t i = 0; i < 3; ++i) {
      const xAOD::Muon *muon = selected_muons.at(i);
      mu.SetPtEtaPhiE(muon->pt() / 1.e3, muon->eta(), muon->phi(),
                      muon->e() / 1.e3);
      myParticles->AddParticle(&mu, mu.Eta(), KLFitter::Particles::kMuon, "",
                               i);
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::add_jets(const std::vector<const xAOD::Jet *> &jets,
                                    KLFitter::Particles *myParticles) {
  if (m_useBtagPriority) {
    ANA_CHECK(setJetskBtagPriority(jets, myParticles, m_njetsRequirement));
  } else {
    ANA_CHECK(setJetskLeadingN(jets, myParticles, m_njetsRequirement));
  }
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::setJetskLeadingN(
    const std::vector<const xAOD::Jet *> &jets,
    KLFitter::Particles *inputParticles, size_t njets) {

  // If container has less jets than required, raise error
  if (m_failOnLessThanXJets) {
    if (jets.size() < njets) {
      ANA_MSG_ERROR("KLFitterTool::setJetskLeadingX: You required "
                    << njets << " jets. Event has " << jets.size() << " jets!");
      return StatusCode::FAILURE;
    }
  }

  size_t index(0);

  for (const xAOD::Jet *jet : jets) {
    if (index > njets - 1)
      break;

    TLorentzVector jet_p4;
    jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(),
                        jet->e() / 1.e3);

    float eff(0), ineff(0);

    if (!m_bTagDecoAcc->isAvailable(*jet)) {
      ANA_MSG_ERROR("RunKLFitterAlg::setJetskLeadingX: jet does not have "
                    << m_bTagDecoration.value() << " aux variable!");
      return StatusCode::FAILURE;
    }

    const bool isTagged = (*m_bTagDecoAcc)(*jet);

    if (m_bTaggingMethodEnum ==
        KLFitter::LikelihoodBase::BtaggingMethod::kWorkingPoint) {
      ANA_CHECK(retrieveEfficiencies(jet, &eff, &ineff))

      inputParticles->AddParticle(
          &jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
          isTagged, eff, 1. / ineff, KLFitter::Particles::kNone);
    } else {
      inputParticles->AddParticle(&jet_p4, jet_p4.Eta(),
                                  KLFitter::Particles::kParton, "", index,
                                  isTagged);
    }
    ++index;
  }
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::retrieveEfficiencies(const xAOD::Jet *jet,
                                                float *eff, float *ineff) {
  // need to make a copy of the jet, so that we can manipulate its flavour to
  // get the various efficiencies
  xAOD::JetContainer jets;
  xAOD::JetAuxContainer jetsAux;
  jets.setStore(&jetsAux);
  xAOD::Jet *jet_copy = new xAOD::Jet();
  jets.push_back(jet_copy);
  *jet_copy = *jet;
  jet_copy->setJetP4(jet->jetP4());
  // treat jet as b-tagged
  jet_copy->setAttribute("HadronConeExclTruthLabelID", 5);
  ANA_CHECK(m_btagging_eff_tool->getMCEfficiency(*jet_copy, *eff));
  // treat jet as light
  jet_copy->setAttribute("HadronConeExclTruthLabelID", 0);
  ANA_CHECK(m_btagging_eff_tool->getMCEfficiency(*jet_copy, *ineff));
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::setJetskBtagPriority(
    const std::vector<const xAOD::Jet *> &jets,
    KLFitter::Particles *inputParticles, const size_t maxJets) {
  // kBtagPriority mode first adds the b jets, then the light jets
  // If your 6th or 7th jet is a b jet, then you probably want this option

  // If container has less jets than required, raise error
  if (m_failOnLessThanXJets) {
    if (jets.size() < maxJets) {
      ANA_MSG_ERROR("KLFitterTool::setJetskBtagPriority: You required "
                    << maxJets << " jets. Event has " << jets.size()
                    << " jets!");
      return StatusCode::FAILURE;
    }
  }

  unsigned int totalJets(0);

  // First find the b-jets
  unsigned int index(0);
  for (const xAOD::Jet *jet : jets) {
    if (totalJets >= maxJets)
      break;

    if (!m_bTagDecoAcc->isAvailable(*jet)) {
      ANA_MSG_ERROR("RunKLFitterAlg::setJetskLeadingX: jet does not have "
                    << m_bTagDecoration.value() << " aux variable!");
      return StatusCode::FAILURE;
    }

    if ((*m_bTagDecoAcc)(*jet)) {
      TLorentzVector jet_p4;
      jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(),
                          jet->e() / 1.e3);

      if (m_bTaggingMethodEnum ==
          KLFitter::LikelihoodBase::BtaggingMethod::kWorkingPoint) {
        float eff(0), ineff(0);
        ANA_CHECK(retrieveEfficiencies(jet, &eff, &ineff));

        inputParticles->AddParticle(
            &jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
            true, eff, 1. / ineff, KLFitter::Particles::kNone);
      } else {
        inputParticles->AddParticle(&jet_p4, jet_p4.Eta(),
                                    KLFitter::Particles::kParton, "", index,
                                    true);
      }
      ++totalJets;
    }  // is b-tagged

    ++index;
  }  // for (jet)

  // Second, find the light jets
  index = 0;
  for (const xAOD::Jet *jet : jets) {
    if (totalJets >= maxJets)
      break;
    if (!(*m_bTagDecoAcc)(*jet)) {
      TLorentzVector jet_p4;
      jet_p4.SetPtEtaPhiE(jet->pt() / 1.e3, jet->eta(), jet->phi(),
                          jet->e() / 1.e3);

      if (m_bTaggingMethodEnum ==
          KLFitter::LikelihoodBase::BtaggingMethod::kWorkingPoint) {
        float eff(0), ineff(0);
        ANA_CHECK(retrieveEfficiencies(jet, &eff, &ineff));

        inputParticles->AddParticle(
            &jet_p4, jet_p4.Eta(), KLFitter::Particles::kParton, "", index,
            false, eff, 1. / ineff, KLFitter::Particles::kNone);
      } else {
        inputParticles->AddParticle(&jet_p4, jet_p4.Eta(),
                                    KLFitter::Particles::kParton, "", index,
                                    false);
      }
      ++totalJets;
    }  // not-btagged jet

    ++index;
  }  // for (jet)
  return StatusCode::SUCCESS;
}

StatusCode RunKLFitterAlg::evaluatePermutations(
    const CP::SystematicSet &sys, const std::vector<size_t> &electron_indices,
    const std::vector<size_t> &muon_indices,
    const std::vector<size_t> &jet_indices) {
  // create or retrieve (if existent) the xAOD::KLFitterResultContainer
  auto resultAuxContainer =
      std::make_unique<xAOD::KLFitterResultAuxContainer>();
  auto resultContainer = std::make_unique<xAOD::KLFitterResultContainer>();
  resultContainer->setStore(resultAuxContainer.get());

  // loop over all permutations
  const int nperm = m_myFitter->Permutations()->NPermutations();
  for (int iperm = 0; iperm < nperm; ++iperm) {
    // Perform the fit
    m_myFitter->Fit(iperm);
    // create a result
    xAOD::KLFitterResult *result = new xAOD::KLFitterResult{};
    resultContainer->push_back(result);

    // Set name hash. This is because it seems std::string is not supported by
    // AuxContainers...
    std::hash<std::string> hash_string;
    result->setSelectionCode(hash_string(sys.name()));

    unsigned int ConvergenceStatusBitWord = m_myFitter->ConvergenceStatus();
    bool MinuitDidNotConverge =
        (ConvergenceStatusBitWord & m_myFitter->MinuitDidNotConvergeMask) != 0;
    bool FitAbortedDueToNaN =
        (ConvergenceStatusBitWord & m_myFitter->FitAbortedDueToNaNMask) != 0;
    bool AtLeastOneFitParameterAtItsLimit =
        (ConvergenceStatusBitWord &
         m_myFitter->AtLeastOneFitParameterAtItsLimitMask) != 0;
    bool InvalidTransferFunctionAtConvergence =
        (ConvergenceStatusBitWord &
         m_myFitter->InvalidTransferFunctionAtConvergenceMask) != 0;

    result->setMinuitDidNotConverge(((MinuitDidNotConverge) ? 1 : 0));
    result->setFitAbortedDueToNaN(((FitAbortedDueToNaN) ? 1 : 0));
    result->setAtLeastOneFitParameterAtItsLimit(
        ((AtLeastOneFitParameterAtItsLimit) ? 1 : 0));
    result->setInvalidTransferFunctionAtConvergence(
        ((InvalidTransferFunctionAtConvergence) ? 1 : 0));

    result->setLogLikelihood(m_myFitter->Likelihood()->LogLikelihood(
        m_myFitter->Likelihood()->GetBestFitParameters()));
    result->setEventProbability(
        std::exp(m_myFitter->Likelihood()->LogEventProbability()));
    result->setParameters(m_myFitter->Likelihood()->GetBestFitParameters());
    result->setParameterErrors(
        m_myFitter->Likelihood()->GetBestFitParameterErrors());

    KLFitter::Particles *myModelParticles =
        m_myFitter->Likelihood()->ParticlesModel();
    KLFitter::Particles **myPermutedParticles =
        m_myFitter->Likelihood()->PParticlesPermuted();

    if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar ||
        m_LHTypeEnum == KLFEnums::Likelihood::ttH ||
        m_LHTypeEnum == KLFEnums::Likelihood::ttbar_JetAngles ||
        m_LHTypeEnum == KLFEnums::Likelihood::ttbar_Angular ||
        m_LHTypeEnum == KLFEnums::Likelihood::ttZTrilepton ||
        m_LHTypeEnum == KLFEnums::Likelihood::ttbar_BoostedLJets) {
      result->setModel_bhad_pt(myModelParticles->Parton(0)->Pt());
      result->setModel_bhad_eta(myModelParticles->Parton(0)->Eta());
      result->setModel_bhad_phi(myModelParticles->Parton(0)->Phi());
      result->setModel_bhad_E(myModelParticles->Parton(0)->E());
      result->setModel_bhad_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(0)));

      result->setModel_blep_pt(myModelParticles->Parton(1)->Pt());
      result->setModel_blep_eta(myModelParticles->Parton(1)->Eta());
      result->setModel_blep_phi(myModelParticles->Parton(1)->Phi());
      result->setModel_blep_E(myModelParticles->Parton(1)->E());
      result->setModel_blep_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(1)));

      result->setModel_lq1_pt(myModelParticles->Parton(2)->Pt());
      result->setModel_lq1_eta(myModelParticles->Parton(2)->Eta());
      result->setModel_lq1_phi(myModelParticles->Parton(2)->Phi());
      result->setModel_lq1_E(myModelParticles->Parton(2)->E());
      result->setModel_lq1_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(2)));

      // boosted likelihood has only one light jet
      if (m_LHTypeEnum != KLFEnums::Likelihood::ttbar_BoostedLJets) {
        result->setModel_lq2_pt(myModelParticles->Parton(3)->Pt());
        result->setModel_lq2_eta(myModelParticles->Parton(3)->Eta());
        result->setModel_lq2_phi(myModelParticles->Parton(3)->Phi());
        result->setModel_lq2_E(myModelParticles->Parton(3)->E());
        result->setModel_lq2_jetIndex(
            jet_indices.at((*myPermutedParticles)->JetIndex(3)));

        if (m_LHTypeEnum == KLFEnums::Likelihood::ttH) {
          result->setModel_Higgs_b1_pt(myModelParticles->Parton(4)->Pt());
          result->setModel_Higgs_b1_eta(myModelParticles->Parton(4)->Eta());
          result->setModel_Higgs_b1_phi(myModelParticles->Parton(4)->Phi());
          result->setModel_Higgs_b1_E(myModelParticles->Parton(4)->E());
          result->setModel_Higgs_b1_jetIndex(
              jet_indices.at((*myPermutedParticles)->JetIndex(4)));

          result->setModel_Higgs_b2_pt(myModelParticles->Parton(5)->Pt());
          result->setModel_Higgs_b2_eta(myModelParticles->Parton(5)->Eta());
          result->setModel_Higgs_b2_phi(myModelParticles->Parton(5)->Phi());
          result->setModel_Higgs_b2_E(myModelParticles->Parton(5)->E());
          result->setModel_Higgs_b2_jetIndex(
              jet_indices.at((*myPermutedParticles)->JetIndex(5)));
        }
      }

      if (m_leptonTypeEnum == KLFEnums::LeptonType::kElectron ||
          m_leptonTypeEnum == KLFEnums::LeptonType::kTriElectron) {
        result->setModel_lep_pt(myModelParticles->Electron(0)->Pt());
        result->setModel_lep_eta(myModelParticles->Electron(0)->Eta());
        result->setModel_lep_phi(myModelParticles->Electron(0)->Phi());
        result->setModel_lep_E(myModelParticles->Electron(0)->E());

        if (m_leptonTypeEnum == KLFEnums::LeptonType::kTriElectron) {
          result->setModel_lep_index(
              electron_indices.at((*myPermutedParticles)->ElectronIndex(0)));

          result->setModel_lepZ1_pt(myModelParticles->Electron(1)->Pt());
          result->setModel_lepZ1_eta(myModelParticles->Electron(1)->Eta());
          result->setModel_lepZ1_phi(myModelParticles->Electron(1)->Phi());
          result->setModel_lepZ1_E(myModelParticles->Electron(1)->E());
          result->setModel_lepZ1_index(
              electron_indices.at((*myPermutedParticles)->ElectronIndex(1)));

          result->setModel_lepZ2_pt(myModelParticles->Electron(2)->Pt());
          result->setModel_lepZ2_eta(myModelParticles->Electron(2)->Eta());
          result->setModel_lepZ2_phi(myModelParticles->Electron(2)->Phi());
          result->setModel_lepZ2_E(myModelParticles->Electron(2)->E());
          result->setModel_lepZ2_index(
              electron_indices.at((*myPermutedParticles)->ElectronIndex(2)));
        }
      }

      if (m_leptonTypeEnum == KLFEnums::LeptonType::kMuon ||
          m_leptonTypeEnum == KLFEnums::LeptonType::kTriMuon) {
        result->setModel_lep_pt(myModelParticles->Muon(0)->Pt());
        result->setModel_lep_eta(myModelParticles->Muon(0)->Eta());
        result->setModel_lep_phi(myModelParticles->Muon(0)->Phi());
        result->setModel_lep_E(myModelParticles->Muon(0)->E());

        if (m_leptonTypeEnum == KLFEnums::LeptonType::kTriMuon) {
          result->setModel_lep_index(
              muon_indices.at((*myPermutedParticles)->MuonIndex(0)));

          result->setModel_lepZ1_pt(myModelParticles->Muon(1)->Pt());
          result->setModel_lepZ1_eta(myModelParticles->Muon(1)->Eta());
          result->setModel_lepZ1_phi(myModelParticles->Muon(1)->Phi());
          result->setModel_lepZ1_E(myModelParticles->Muon(1)->E());
          result->setModel_lepZ1_index(
              muon_indices.at((*myPermutedParticles)->MuonIndex(1)));

          result->setModel_lepZ2_pt(myModelParticles->Muon(2)->Pt());
          result->setModel_lepZ2_eta(myModelParticles->Muon(2)->Eta());
          result->setModel_lepZ2_phi(myModelParticles->Muon(2)->Phi());
          result->setModel_lepZ2_E(myModelParticles->Muon(2)->E());
          result->setModel_lepZ2_index(
              muon_indices.at((*myPermutedParticles)->MuonIndex(2)));
        }
      }

      result->setModel_nu_pt(myModelParticles->Neutrino(0)->Pt());
      result->setModel_nu_eta(myModelParticles->Neutrino(0)->Eta());
      result->setModel_nu_phi(myModelParticles->Neutrino(0)->Phi());
      result->setModel_nu_E(myModelParticles->Neutrino(0)->E());
    } else if (m_LHTypeEnum == KLFEnums::Likelihood::ttbar_AllHad) {
      result->setModel_b_from_top1_pt(myModelParticles->Parton(0)->Pt());
      result->setModel_b_from_top1_eta(myModelParticles->Parton(0)->Eta());
      result->setModel_b_from_top1_phi(myModelParticles->Parton(0)->Phi());
      result->setModel_b_from_top1_E(myModelParticles->Parton(0)->E());
      result->setModel_b_from_top1_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(0)));

      result->setModel_b_from_top2_pt(myModelParticles->Parton(1)->Pt());
      result->setModel_b_from_top2_eta(myModelParticles->Parton(1)->Eta());
      result->setModel_b_from_top2_phi(myModelParticles->Parton(1)->Phi());
      result->setModel_b_from_top2_E(myModelParticles->Parton(1)->E());
      result->setModel_b_from_top2_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(1)));

      result->setModel_lj1_from_top1_pt(myModelParticles->Parton(2)->Pt());
      result->setModel_lj1_from_top1_eta(myModelParticles->Parton(2)->Eta());
      result->setModel_lj1_from_top1_phi(myModelParticles->Parton(2)->Phi());
      result->setModel_lj1_from_top1_E(myModelParticles->Parton(2)->E());
      result->setModel_lj1_from_top1_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(2)));

      result->setModel_lj2_from_top1_pt(myModelParticles->Parton(3)->Pt());
      result->setModel_lj2_from_top1_eta(myModelParticles->Parton(3)->Eta());
      result->setModel_lj2_from_top1_phi(myModelParticles->Parton(3)->Phi());
      result->setModel_lj2_from_top1_E(myModelParticles->Parton(3)->E());
      result->setModel_lj2_from_top1_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(3)));

      result->setModel_lj1_from_top2_pt(myModelParticles->Parton(4)->Pt());
      result->setModel_lj1_from_top2_eta(myModelParticles->Parton(4)->Eta());
      result->setModel_lj1_from_top2_phi(myModelParticles->Parton(4)->Phi());
      result->setModel_lj1_from_top2_E(myModelParticles->Parton(4)->E());
      result->setModel_lj1_from_top2_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(4)));

      result->setModel_lj2_from_top2_pt(myModelParticles->Parton(5)->Pt());
      result->setModel_lj2_from_top2_eta(myModelParticles->Parton(5)->Eta());
      result->setModel_lj2_from_top2_phi(myModelParticles->Parton(5)->Phi());
      result->setModel_lj2_from_top2_E(myModelParticles->Parton(5)->E());
      result->setModel_lj2_from_top2_jetIndex(
          jet_indices.at((*myPermutedParticles)->JetIndex(5)));
    }
  }  // Loop over permutations

  // Normalize event probability to unity
  // work out best permutation
  float sumEventProbability(0.), bestEventProbability(0.);
  size_t bestPermutation(999), iPerm(0);

  // First loop
  for (auto x : *resultContainer) {
    float prob = x->eventProbability();
    short minuitDidNotConverge = x->minuitDidNotConverge();
    short fitAbortedDueToNaN = x->fitAbortedDueToNaN();
    short atLeastOneFitParameterAtItsLimit =
        x->atLeastOneFitParameterAtItsLimit();
    short invalidTransferFunctionAtConvergence =
        x->invalidTransferFunctionAtConvergence();
    sumEventProbability += prob;
    ++iPerm;

    // check if the best value has the highest event probability AND converged
    if (minuitDidNotConverge)
      continue;
    if (fitAbortedDueToNaN)
      continue;
    if (atLeastOneFitParameterAtItsLimit)
      continue;
    if (invalidTransferFunctionAtConvergence)
      continue;

    if (prob > bestEventProbability) {
      bestEventProbability = prob;
      // Using iPerm -1 because it has already been incremented before
      bestPermutation = iPerm - 1;
    }
  }

  // Second loop
  iPerm = 0;
  for (auto x : *resultContainer) {
    x->setEventProbability(x->eventProbability() / sumEventProbability);
    if (iPerm == bestPermutation) {
      x->setBestPermutation(1);
    } else {
      x->setBestPermutation(0);
    }
    ++iPerm;
  }

  // Save all permutations
  if (m_saveAllPermutations) {
    ANA_CHECK(m_outHandle.record(std::move(resultContainer),
                                 std::move(resultAuxContainer), sys));
  } else {  // Save only the best permutation
    // create or retrieve the xAOD::KLFitterResultContainer
    auto bestContainer = std::make_unique<xAOD::KLFitterResultContainer>();
    auto bestAuxContainer =
        std::make_unique<xAOD::KLFitterResultAuxContainer>();
    bestContainer->setStore(bestAuxContainer.get());

    for (auto x : *resultContainer) {
      if (x->bestPermutation() == 1) {
        xAOD::KLFitterResult *result = new xAOD::KLFitterResult{};
        result->makePrivateStore(*x);
        bestContainer->push_back(result);
      }
    }
    ANA_CHECK(m_outHandle.record(std::move(bestContainer),
                                 std::move(bestAuxContainer), sys));
  }

  return StatusCode::SUCCESS;
}

}  // namespace EventReco
