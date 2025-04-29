/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH__PARTICLELEVEL_OVERLAPREMOVAL__ALG_H
#define TRUTH__PARTICLELEVEL_OVERLAPREMOVAL__ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <SelectionHelpers/SelectionReadHandle.h>
#include <xAODJet/JetContainer.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace CP {
class ParticleLevelOverlapRemovalAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{
      this, "jets", "", "the name of the input truth jet container"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_electronsKey{
      this, "electrons", "", "the name of the input truth electrons container"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_muonsKey{
      this, "muons", "", "the name of the input truth muons container"};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_photonsKey{
      this, "photons", "", "the name of the input truth photons container"};
  SG::WriteDecorHandleKey<xAOD::JetContainer> m_decORjet{
      this, "decORjet", "SetMe", ""};
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decORelectron{
      this, "decORelectron", "TruthElectrons.passesOR", ""};
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decORmuon{
      this, "decORmuon", "TruthMuons.passesOR", ""};
  SG::WriteDecorHandleKey<xAOD::TruthParticleContainer> m_decORphoton{
      this, "decORphoton", "TruthPhotons.passesOR", ""};
  CP::SelectionReadHandle m_jetSelection{
      this, "jetSelection", "", "the selection on the input truth jets"};
  CP::SelectionReadHandle m_electronSelection{
      this, "electronSelection", "",
      "the selection on the input truth electrons"};
  CP::SelectionReadHandle m_muonSelection{
      this, "muonSelection", "", "the selection on the input truth muons"};
  CP::SelectionReadHandle m_photonSelection{
      this, "photonSelection", "", "the selection on the input truth photons"};
  Gaudi::Property<bool> m_useRapidity{
      this, "useRapidityForDeltaR", true,
      "whether to use rapidity instead of pseudo-rapidity for the calculation "
      "of DeltaR"};
  Gaudi::Property<bool> m_useDressedProperties{
      this, "useDressedProperties", true,
      "whether to use dressed electron and muon kinematics rather than simple "
      "P4 kinematics"};
  Gaudi::Property<bool> m_doJetElectronOR{
      this, "doJetElectronOR", false,
      "whether to perform jet-electron overlap removal"};
  Gaudi::Property<bool> m_doJetMuonOR{
      this, "doJetMuonOR", false,
      "whether to perform jet-muon overlap removal"};
  Gaudi::Property<bool> m_doJetPhotonOR{
      this, "doJetPhotonOR", false,
      "whether to perform jet-photon overlap removal"};
  Gaudi::Property<std::string> m_decLabelOR{
      this, "labelOR", "passesOR",
      "decoration to apply to all particles for overlap removal"};

  float dressedDeltaR(const xAOD::Jet* p1, TLorentzVector& p2,
                      bool useRapidity) const;
};

}  // namespace CP

#endif
