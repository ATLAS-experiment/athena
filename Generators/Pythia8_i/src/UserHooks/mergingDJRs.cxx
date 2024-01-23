/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "Pythia8_i/UserHooksFactory.h"
#include "UserHooksUtils.h"
#include "UserSetting.h"

// ROOT, for saving file.
#include "TFile.h"

// ROOT, for histogramming.
#include "TF1.h"
#include "TH1.h"

#include <iostream>

namespace Pythia8 {

// Class to compute the DJRs
class mergingDJRs : public UserHooks {

private:
  // Settings for SlowJet jet finder, with kT clustering
  int m_power = 1;
  double m_etaMax = 10.;
  double m_radius = 1.0;
  double m_pTjetMin = 10.;
  Pythia8::SlowJetHook *m_sjHookPtrIn = 0;
  bool m_useFJcoreIn = false; // SHOULD BE FALSE TO ALLOW THE SUB PROCESS BEFORE
                              // CLUSTERING THE PAARTICLES
  bool m_useStandardRin = true;
  int m_nSel = 2; // Exclude neutrinos (and other invisible) from study.
  int m_massSetIn = 1;
  std::unique_ptr<Pythia8::SlowJet> m_slowJet;

  // ROOT output files
  std::unique_ptr<TH1F> m_HistDjr, m_HistDjr2;
  std::unique_ptr<TFile> m_outFile;

  vector<double> m_result, m_DJR;
  Event m_workEventJet;

public:
  mergingDJRs() {
    // Slowjet pointer
    m_slowJet = std::make_unique<Pythia8::SlowJet>(
        m_power, m_radius, m_pTjetMin, m_etaMax, m_nSel, m_massSetIn,
        m_sjHookPtrIn, m_useFJcoreIn, m_useStandardRin);

    // ROOT histograms and the output file where to save them
    m_HistDjr = std::make_unique<TH1F>("HistDjr", "The first DJR", 100, 0.0, 3.0);
    m_HistDjr2 =
        std::make_unique<TH1F>("HistDjr2", "The second DJR", 100, 0.0, 3.0);
    m_outFile = std::make_unique<TFile>("hist-DJR.root", "RECREATE");

    std::cout << "**********************************************************"
              << std::endl;
    std::cout << "*                                                        *"
              << std::endl;
    std::cout << "*	  the jet merging userhook CKKWL DJRS is working   *"
              << std::endl;
    std::cout << "*                                                        *"
              << std::endl;
    std::cout << "**********************************************************"
              << std::endl;
  }

  // Destructor prints histogram
  ~mergingDJRs() {
    m_HistDjr->Write();
    m_HistDjr2->Write();
  }

  // Parton level vetoing
  virtual bool canVetoPartonLevel() override { return true; }
  virtual bool doVetoPartonLevel(const Event &event) override;

  // Function to compute the DJRs
  virtual void getDJR(const Event &event);

  // To initialize the variables
  virtual bool initAfterBeams() override {

    m_workEventJet.init("(workEventJet)", particleDataPtr);

    return true;
  }
};

// parton level veto (before beam remnants and resonance showers)
inline bool mergingDJRs::doVetoPartonLevel(const Event &event) {

  // subEvent method extract a list of the current partons list and save the
  // output in workEvent.
  subEvent(event);
  m_workEventJet = workEvent;

  // The selected particles to pass to slowjet
  for (int j = 0; j < m_workEventJet.size(); ++j) {
    if (!(m_workEventJet[j].isFinal()) || m_workEventJet[j].isLepton() ||
        m_workEventJet[j].id() == 23 || std::abs(m_workEventJet[j].id()) == 24 ||
        m_workEventJet[j].id() == 22) {
      m_workEventJet[j].statusNeg();
      continue;
    }
  }

  // slowjet analyze the events
  if (not m_slowJet->setup(m_workEventJet)){
    std::cout<< "setup failed in  mergingDJRs::doVetoPartonLevel\n";
    return false;
  }

  // Call getDJR and store the DJRs vector
  getDJR(m_workEventJet);

  // if we reached here then no veto
  return false;
}

// Compute DJR vector
inline void mergingDJRs::getDJR(const Event &event) {

  // setup slowjet pointer
  if (not m_slowJet->setup(event)){
    std::cout<< "setup failed in  mergingDJRs::getDJR\n";
    return;
  };

  // Clear members.
  m_DJR.clear();
  m_result.clear();

  while (m_slowJet->sizeAll() - m_slowJet->sizeJet() > 0) {
    // Save the next clustering scale.
    m_result.push_back(sqrt(m_slowJet->dNext()));
    // Perform step.
    m_slowJet->doStep();
  }

  // Save clustering scales in reverse order
  for (int i = m_result.size() - 1; i >= 0; --i) {
    m_DJR.push_back(m_result[i]);
  }

  // Fill the histogram and Normalize them
  double eventWeight = infoPtr->mergingWeight() * infoPtr->weight();

  if (m_DJR.size() > 0) {
    m_HistDjr->Fill(log10(m_DJR[0]), eventWeight);
    m_HistDjr2->Fill(log10(m_DJR[1]), eventWeight);
  }
}

Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::mergingDJRs>
    Ckkwl("mergingDJRs");

} // namespace Pythia8
