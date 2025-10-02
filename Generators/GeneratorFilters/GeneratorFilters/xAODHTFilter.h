/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORFILTERS_XAODHTFilter_H
#define GENERATORFILTERS_XAODHTFilter_H

#include "GeneratorModules/GenFilter.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
// Defs for the particle origin
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "xAODEventInfo/EventInfo.h"
#include "GaudiKernel/SystemOfUnits.h"

class MsgStream;
class StoreGateSvc;


class xAODHTFilter:public GenFilter {

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterFinalize() override final;
  virtual StatusCode filterEvent() override final;

private:

  Gaudi::Property<double> m_MinJetPt{this, "MinJetPt", 0 * Gaudi::Units::GeV};  //!< Min pT for the truth jets
  Gaudi::Property<double> m_MaxJetEta{this, "MaxJetEta", 10.0}; //!< Max eta for the truth jets
  Gaudi::Property<double> m_MinHT{this, "MinHT", 20. * Gaudi::Units::GeV};  //!< Min HT for events
  Gaudi::Property<double> m_MaxHT{this, "MaxHT", 14000. * Gaudi::Units::GeV};  //!< Max HT for events
  Gaudi::Property<double> m_MinLepPt{this, "MinLeptonPt", 0 * Gaudi::Units::GeV};  //!< Min pT for the truth jets
  Gaudi::Property<double> m_MaxLepEta{this, "MaxLeptonEta", 10.0}; //!< Max eta for the truth jets
  Gaudi::Property<bool> m_UseNu{this, "UseNeutrinosFromWZTau", false, "Include neutrinos from W/Z/tau decays in the calculation of HT"};  //!< Use neutrinos in HT
  Gaudi::Property<bool> m_UseLep{this, "UseLeptonsFromWZTau", false, "Include e/mu from W/Z/tau decays in the HT"};   //!< Use leptons in HT

  Gaudi::Property<std::string> m_eventInfoName{this, "EventInfoName", "EventInfo"};

  long m_total{};    //!< Total number of events tested
  long m_passed{};   //!< Number of events passing all cuts
  long m_ptfailed{}; //!< Number of events failing the pT cuts

  PublicToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", "MCTruthClassifier/DFCommonTruthClassifier"};

  SG::ReadHandleKey<xAOD::JetContainer> m_TruthJetContainerName{this, "TruthJetContainer", "AntiKt4TruthWZJets"}; // Name of the truth jet container
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  SG::WriteDecorHandleKey<xAOD::EventInfo> m_mcFilterHTKey {this
    , "mcFilterHTKey"
    , "TMPEvtInfo.mcFilterHTKey"
    , "Decoration for MC Filter HT"};

};

#endif
