/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// ----------------------------------------------
//
//  xAODBSignalFilter.h
//
//  Author:      Malte Muller, August 2002
//  Modified by: Cristiano Alpigiani
//               <Cristiano.Alpigiani@cern.ch>,
//               September 2013
//
// ----------------------------------------------
#ifndef GENERATORFILTERSXAODBSIGNALFILTER_H
#define GENERATORFILTERSXAODBSIGNALFILTER_H

#include "GeneratorModules/GenFilter.h"
#include "GaudiKernel/NTuple.h"
#include <vector>

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "TTree.h"
#include "TLorentzVector.h"

#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"

class xAODBSignalFilter : public GenFilter
{

public:
  using GenFilter::GenFilter;

  virtual StatusCode filterInitialize() override final;
  virtual StatusCode filterEvent() override final;
  virtual StatusCode filterFinalize() override final;

private:

  // ** Private data members **

  // ** Declare the algorithm's properties **
  // ** For cuts on final particle pT and eta **
  Gaudi::Property<bool> m_localLVL1MuonCutOn{this, "LVL1MuonCutOn", false};
  Gaudi::Property<bool> m_localLVL2MuonCutOn{this, "LVL2MuonCutOn", false};
  Gaudi::Property<bool> m_localLVL2ElectronCutOn{this, "LVL2ElectronCutOn", false};
  Gaudi::Property<double> m_localLVL1MuonCutPT{this, "LVL1MuonCutPT", 0.0};
  Gaudi::Property<double> m_localLVL1MuonCutEta{this, "LVL1MuonCutEta", 102.5};
  Gaudi::Property<double> m_localLVL2MuonCutPT{this, "LVL2MuonCutPT", 0.0};
  Gaudi::Property<double> m_localLVL2MuonCutEta{this, "LVL2MuonCutEta", 102.5};
  Gaudi::Property<double> m_localLVL2ElectronCutPT{this, "LVL2ElectronCutPT", 0.0};
  Gaudi::Property<double> m_localLVL2ElectronCutEta{this, "LVL2ElectronCutEta", 102.5};
  Gaudi::Property<bool> m_cuts_f_e_on{this, "Cuts_Final_e_switch", false};
  Gaudi::Property<double> m_cuts_f_e_pT{this, "Cuts_Final_e_pT", 0.};
  Gaudi::Property<double> m_cuts_f_e_eta{this, "Cuts_Final_e_eta", 2.5}; // FIXME why is this default different from the rest of the eta cuts?
  Gaudi::Property<bool> m_cuts_f_mu_on{this, "Cuts_Final_mu_switch", false};
  Gaudi::Property<double> m_cuts_f_mu_pT{this, "Cuts_Final_mu_pT", 0.};
  Gaudi::Property<double> m_cuts_f_mu_eta{this, "Cuts_Final_mu_eta", 102.5};
  Gaudi::Property<bool> m_cuts_f_had_on{this, "Cuts_Final_hadrons_switch", false};
  Gaudi::Property<double> m_cuts_f_had_pT{this, "Cuts_Final_hadrons_pT", 0.};
  Gaudi::Property<double> m_cuts_f_had_eta{this, "Cuts_Final_hadrons_eta", 102.5};
  Gaudi::Property<bool> m_cuts_f_gam_on{this, "Cuts_Final_gamma_switch", false};
  Gaudi::Property<double> m_cuts_f_gam_pT{this, "Cuts_Final_gamma_pT",  0.};
  Gaudi::Property<double> m_cuts_f_gam_eta{this, "Cuts_Final_gamma_eta", 102.5};
  Gaudi::Property<bool> m_cuts_f_K0_on{this, "Cuts_Final_K0_switch", false};
  Gaudi::Property<double>  m_cuts_f_K0_pT{this, "Cuts_Final_K0_pT", 0.};
  Gaudi::Property<double> m_cuts_f_K0_eta{this, "Cuts_Final_K0_eta", 102.5};
  // ** Declare the signal B-meson/hadron PDGid **
  Gaudi::Property<int> m_B_pdgid{this, "B_PDGCode", 0};       // pdgID of the mother
  // ** Declare properties for mass filter **
  Gaudi::Property<bool> m_InvMass_switch{this, "InvMass_switch", false};
  Gaudi::Property<int> m_InvMass_PartId1{this, "InvMass_PartId1", 13}; // pdgID of the couple used for mass cuts
  Gaudi::Property<int> m_InvMass_PartId2{this, "InvMass_PartId2", -13}; // pdgID of the couple used for mass cuts
  Gaudi::Property<double> m_InvMass_PartFakeMass1{this, "InvMass_PartFakeMass1", -1.};
  Gaudi::Property<double> m_InvMass_PartFakeMass2{this, "InvMass_PartFakeMass2", -1.};
  // Mass range for invariant mass cut
  Gaudi::Property<double> m_InvMassMin{this, "InvMassMin", 0.0};
  Gaudi::Property<double> m_InvMassMax{this, "InvMassMax", 14000000.0};
  Gaudi::Property<bool> m_TotalInvMass_switch{this, "TotalInvMass_switch", false};
  Gaudi::Property<double> m_TotalInvMassMin{this, "TotalInvMassMin", 0.0};
  Gaudi::Property<double> m_TotalInvMassMax{this, "TotalInvMassMax", 14000000.0};
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthPartContKey{this, "TruthParticleContainerKey", "TruthGen"};

  // Event counters
  int    m_EventCnt{};
  double m_LVL1Counter{};     // Counting variable for events passing LVL1
  double m_LVL2Counter{};     // Counting variable for events passing LVL2
  double m_rejectedTrigger{}; // Failed to pass trigger
  double m_rejectedAll{};     // Failed to pass filter (both trigger and signal selection)

  // ** Private member functions **

  // Find child
  void FindAllChildren(const xAOD::TruthParticle* mother,std::string treeIDStr,
                       bool fromFinalB, bool &foundSignal, bool &passedAllCuts,
                       TLorentzVector &p1, TLorentzVector &p2, bool fromSelectedB,
                       TLorentzVector &total_4mom) const;

  // Check whether child has pass cuts
  bool FinalStatePassedCuts(const xAOD::TruthParticle* child) const;

  // Test whether final states pass cuts
  bool test_cuts(const double myPT, const double testPT,
                 const double myEta, const double testEta) const;

  // LVL1 and LVL2 cuts
  bool LVL1_Mu_Trigger(const xAOD::TruthParticle* child) const;
  bool LVL2_eMu_Trigger(const xAOD::TruthParticle* child) const;

  // Print child (for debug)
  void PrintChild(const xAOD::TruthParticle* child, const std::string& treeIDStr, const bool fromFinalB) const;

};


#endif
