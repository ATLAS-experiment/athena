/*
    Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Oliver Majersky
/// @author Baptiste Ravina

#ifndef KLFITTERANALYSISALGORITHMS_RUNKLFITTERALG_H_
#define KLFITTERANALYSISALGORITHMS_RUNKLFITTERALG_H_

#include <algorithm>
#include <numeric>
#include <optional>

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>

// Framework includes
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODJet/JetContainer.h>
#include <xAODMissingET/MissingETContainer.h>
#include <xAODMuon/MuonContainer.h>

#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "xAODJet/JetAuxContainer.h"

// Externals
#include "KLFitterAnalysisAlgorithms/KLFitterEnums.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResultAuxContainer.h"
#include "KLFitterAnalysisAlgorithms/KLFitterResultContainer.h"
#include "KLFitter/BoostedLikelihoodTopLeptonJets.h"
#include "KLFitter/DetectorAtlas_8TeV.h"
#include "KLFitter/Fitter.h"
#include "KLFitter/LikelihoodTTHLeptonJets.h"
#include "KLFitter/LikelihoodTTZTrilepton.h"
#include "KLFitter/LikelihoodTopAllHadronic.h"
#include "KLFitter/LikelihoodTopLeptonJets.h"
#include "KLFitter/LikelihoodTopLeptonJets_Angular.h"
#include "KLFitter/LikelihoodTopLeptonJets_JetAngles.h"
#include "KLFitter/Permutations.h"

namespace EventReco {

class RunKLFitterAlg final : public EL::AnaAlgorithm {

 public:
  using EL::AnaAlgorithm::AnaAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext& ctx) final;

 private:
  StatusCode execute_syst(const CP::SystematicSet &sys,
                          const EventContext &ctx);
  StatusCode add_leptons(
      const std::vector<const xAOD::Electron *> &selected_electrons,
      const std::vector<const xAOD::Muon *> &selected_muons,
      KLFitter::Particles *myParticles);

  StatusCode add_jets(const std::vector<const xAOD::Jet *> &selected_jets,
                      KLFitter::Particles *myParticles);

  StatusCode setJetskLeadingN(const std::vector<const xAOD::Jet *> &jets,
                              KLFitter::Particles *inputParticles,
                              const size_t njets);

  StatusCode getBTagDecision(const xAOD::Jet &jet, bool &isTagged) const;

  StatusCode addJet(const xAOD::Jet *jet, const size_t index,
                    const bool isTagged, KLFitter::Particles *inputParticles);

  StatusCode retrieveEfficiencies(const xAOD::Jet *jet, float *eff,
                                  float *ineff);

  StatusCode setJetskBtagPriority(const std::vector<const xAOD::Jet *> &jets,
                                  KLFitter::Particles *inputParticles,
                                  const size_t maxJets);

  StatusCode evaluatePermutations(const CP::SystematicSet &sys,
                                  const EventContext &ctx,
                                  const std::vector<size_t> &electron_indices,
                                  const std::vector<size_t> &muon_indices,
                                  const std::vector<size_t> &jet_indices);

  template <typename T>
  std::vector<const T *> sortPt(const std::vector<const T *> &particles,
                                std::vector<size_t> &indices) {
    indices.resize(particles.size());
    std::iota(indices.begin(), indices.end(), size_t{0});
    std::sort(indices.begin(), indices.end(),
              [&particles](const size_t i, const size_t j) {
                return particles[i]->pt() > particles[j]->pt();
              });
    std::vector<const T *> sorted_particles;
    sorted_particles.reserve(particles.size());
    for (const size_t i : indices) {
      sorted_particles.push_back(particles[i]);
    }
    return sorted_particles;
  }

  // systematics
  CP::SysListHandle m_systematicsList{this};

  // inputs needed for reconstruction
  CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle{
      this, "electrons", "", "the electron container to use"};
  CP::SysReadSelectionHandle m_electronSelection{
      this, "electronSelection", "", "the selection on the input electrons"};

  CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle{
      this, "muons", "", "the muon container to use"};
  CP::SysReadSelectionHandle m_muonSelection{
      this, "muonSelection", "", "the selection on the input muons"};

  CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle{
      this, "jets", "", "the jet container to use"};
  CP::SysReadSelectionHandle m_jetSelection{this, "jetSelection", "",
                                            "the selection on the input jets"};

  CP::SysReadHandle<xAOD::MissingETContainer> m_metHandle{
      this, "met", "", "the MET container to use"};

  CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle{
      this, "eventInfo", "EventInfo",
      "the EventInfo container to read selection decisions from"};

  // output container
  CP::SysWriteHandle<xAOD::KLFitterResultContainer,
                     xAOD::KLFitterResultAuxContainer>
      m_outHandle{this, "result", "KLFitterResult_%SYS%",
                  "the output KLFitterResultContainer"};

  CP::SysReadSelectionHandle m_selection{this, "selectionDecorationName", "",
                                         "Name of the selection on which this "
                                         "KLFitter instance is allowed to run"};

  // configurable properties
  Gaudi::Property<std::string> m_leptonType{this, "LeptonType", "kUndefined",
                                            "Define the lepton type"};
  Gaudi::Property<std::string> m_LHType{this, "LHType", "kUndefined",
                                        "Define the Likelihood type"};
  Gaudi::Property<std::string> m_transferFunctionsPath{
      this, "TransferFunctionsPath",
      "dev/AnalysisTop/KLFitterTFs/mc12a/akt4_LCtopo_PP6/",
      "Path to transfer functions"};
  Gaudi::Property<std::string> m_jetSelectionMode{
      this, "JetSelectionMode", "kBtagPriorityFourJets",
      "Define the behavior for selecting jets"};
  Gaudi::Property<std::string> m_bTaggingMethod{
      this, "BTaggingMethod", "kNotag",
      "Method for accounting b-tagging information"};
  Gaudi::Property<std::string> m_bTagDecoration{
      this, "BTaggingDecoration", "",
      "Name of the btag decision decoration for jets"};
  Gaudi::Property<std::string> m_METterm{this, "METterm", "Final",
                                         "Which MET term should be used"};
  Gaudi::Property<float> m_massTop{
      this, "TopMass", 172.5,
      "The mass of top quark used in KLFitter likelihood (assuming the fixed "
      "m_top mode is used)"};
  Gaudi::Property<bool> m_fixedTopMass{
      this, "TopMassFixed", true,
      "If the top quark mass is fixed in the likelihood to the value of "
      "TopMass parameter"};
  Gaudi::Property<bool> m_saveAllPermutations{
      this, "SaveAllPermutations", false,
      "Whether to store only the permutation with highest KLFitter event "
      "probability, or all"};
  Gaudi::Property<bool> m_failOnLessThanXJets{
      this, "FailOnLessThanXJets", false,
      "Fail if kLeadingX or kBtagPriorityXJets is set and the number of jets "
      "in the event is less than X"};

  KLFEnums::LeptonType m_leptonTypeEnum{};
  KLFEnums::Likelihood m_LHTypeEnum{};
  KLFEnums::JetSelectionMode m_jetSelectionModeEnum{};

  bool m_useBtagPriority{false};
  size_t m_njetsRequirement{0};

  std::unique_ptr<KLFitter::Fitter> m_myFitter;
  std::unique_ptr<KLFitter::DetectorAtlas_8TeV> m_myDetector;
  KLFitter::LikelihoodBase::BtaggingMethod m_bTaggingMethodEnum{};

  std::unique_ptr<KLFitter::LikelihoodBase> m_likelihood;

  ToolHandle<IBTaggingEfficiencyTool> m_btagging_eff_tool{
      this, "btagEffTool", "", "the b-tagging efficiency tool"};

  std::optional<SG::ConstAccessor<char>> m_bTagDecoAcc;

  // single-jet container used to query the b-tagging efficiencies
  xAOD::JetAuxContainer m_effJetsAux;
  xAOD::JetContainer m_effJets;
  xAOD::Jet *m_effJet{nullptr};
};
}  // namespace EventReco

#endif
