/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERSXAODVBFFORWARDJETSFILTER_H
#define GENERATORFILTERSXAODVBFFORWARDJETSFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "CLHEP/Vector/LorentzVector.h"
#include <vector>

/// Filter of the type of VBF forward jets
/// @author Junichi TANAKA
class xAODVBFForwardJetsFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  Gaudi::Property<double> m_JetMinPt{this, "JetMinPt", 10. * Gaudi::Units::GeV};
  Gaudi::Property<double> m_JetMaxEta{this, "JetMaxEta", 5.};
  Gaudi::Property<int> m_NJets{this, "NJets", 2};
  Gaudi::Property<double> m_Jet1MinPt{this, "Jet1MinPt", 20. * Gaudi::Units::GeV};
  Gaudi::Property<double> m_Jet1MaxEta{this, "Jet1MaxEta", 5.};
  Gaudi::Property<double> m_Jet2MinPt{this, "Jet2MinPt", 10. * Gaudi::Units::GeV};
  Gaudi::Property<double> m_Jet2MaxEta{this, "Jet2MaxEta", 5.};
  Gaudi::Property<bool> m_UseOppositeSignEtaJet1Jet2{this, "UseOppositeSignEtaJet1Jet2", false};
  Gaudi::Property<double> m_DeltaEtaJJ{this, "DeltaEtaJJ", 2.0};
  Gaudi::Property<double> m_DeltaPhiJJ{this, "DeltaPhiJJ", -1.0};
  Gaudi::Property<bool> m_RequireSamePair{this, "RequireSamePair", false};
  Gaudi::Property<double> m_MassJJ{this, "MassJJ", 300. * Gaudi::Units::GeV};
  Gaudi::Property<bool> m_UseLeadingJJ{this, "UseLeadingJJ", false};
  Gaudi::Property<double> m_LGMinPt{this, "LGMinPt", 10. * Gaudi::Units::GeV};
  Gaudi::Property<double> m_LGMaxEta{this, "LGMaxEta", 2.5};
  Gaudi::Property<double> m_DeltaRJLG{this, "DeltaRJLG", 0.05};
  Gaudi::Property<double> m_RatioPtJLG{this, "RatioPtJLG", 0.3};

  SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainer", "AntiKt4TruthJets"}; // Name of the truth jet container
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  CLHEP::HepLorentzVector sumDaughterNeutrinos(const xAOD::TruthParticle* tau);
  void removePseudoJets(std::vector<const xAOD::Jet*>& jetList,
                        std::vector<const xAOD::TruthParticle*>& MCTruthPhotonList,
                        std::vector<const xAOD::TruthParticle*>& MCTruthElectronList,
                        std::vector<CLHEP::HepLorentzVector>  & MCTruthTauList);
  double getMinDeltaR(const xAOD::Jet* jet, std::vector<const xAOD::TruthParticle*>& list);
  double getMinDeltaR(const xAOD::Jet* jet, std::vector<CLHEP::HepLorentzVector>& list);
};

#endif
