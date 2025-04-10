/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERSXAODVBFMJJINTERVALFILTER_H
#define GENERATORFILTERSXAODVBFMJJINTERVALFILTER_H

#include "AthContainers/ConstDataVector.h"
#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace CLHEP {
  class HepRandomEngine;
}

class xAODVBFMjjIntervalFilter : public GenFilter {
public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;

private:

  CLHEP::HepRandomEngine* getRandomEngine(const std::string& streamName,
                                          const EventContext& ctx) const;

  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};// Random number generator

  Gaudi::Property<double> m_olapPt{this, "MinOverlapPT", 15.0 * Gaudi::Units::GeV};
  Gaudi::Property<double> m_yMax{this, "RapidityAcceptance", 5.0};
  Gaudi::Property<double> m_pTavgMin{this, "MinSecondJetPT", 15.0 * Gaudi::Units::GeV};// Required average dijet pT
  SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainerName", "AntiKt4TruthJets"}; // Name of the truth jet container
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  Gaudi::Property<double> m_prob0{this, "NoJetProbability", 0.0002};
  Gaudi::Property<double> m_prob1{this, "OneJetProbability", 0.001};
  Gaudi::Property<double> m_prob2low{this, "LowMjjProbability", 0.005};
  Gaudi::Property<double> m_prob2high{this, "HighMjjProbability", 1.0};
  Gaudi::Property<double> m_mjjlow{this, "LowMjj", 100.0 * Gaudi::Units::GeV};
  Gaudi::Property<bool> m_truncatelowmjj{this, "TruncateAtLowMjj", false};
  Gaudi::Property<double> m_mjjhigh{this, "HighMjj", 800.0 * Gaudi::Units::GeV};
  Gaudi::Property<bool> m_truncatehighmjj{this, "TruncateAtHighMjj", false};
  Gaudi::Property<bool> m_photonjetoverlap{this, "PhotonJetOverlapRemoval", false};
  Gaudi::Property<bool> m_electronjetoverlap{this, "ElectronJetOverlapRemoval", true};
  Gaudi::Property<bool> m_taujetoverlap{this, "TauJetOverlapRemoval", false};
  Gaudi::Property<bool> m_ApplyNjet{this, "ApplyNjet", false};
  Gaudi::Property<unsigned int> m_NJetsMin{this, "Njets", 2};
  Gaudi::Property<unsigned int> m_NJetsMax{this, "NjetsMax", -1};
  Gaudi::Property<bool> m_ApplyWeighting{this, "ApplyWeighting", true};
  Gaudi::Property<bool> m_applyDphi{this, "ApplyDphi", false};
  Gaudi::Property<double> m_dphijj{this, "dphijjMax", 2.5};
  double m_alpha{0.}; // FIXME configured value overridden in filterInitialize() function
  double m_norm{1.0}; // Normalization for weights //< @todo Scalefactor always set to 1.0! Remove?


  bool checkOverlap(double, double, const std::vector<const xAOD::TruthParticle*>&);
  bool checkOverlap(double, double, const std::vector<TLorentzVector>&);
  TLorentzVector sumDaughterNeutrinos( const xAOD::TruthParticle* );

public:

  bool ApplyMassDphi(ConstDataVector<xAOD::JetContainer> *jets);
  double getEventWeight(ConstDataVector<xAOD::JetContainer> *jets) const;
};

#endif
