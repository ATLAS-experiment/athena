/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_LARGERJET_H
#define TRIGGEPPERF_LARGERJET_H

#include "TLorentzVector.h"
#include <vector>

namespace Gep {

  // Output object for the JetTaggerLRJMaker (modified seeded-cone large-R jet).
  struct LargeRJet
  {
    TLorentzVector vec;

    // Seed kinematics (undigitised).
    float seedEt {0};
    float seedEta {0};
    float seedPhi {0};

    // Jet radius
    float radius {0};

    // Number of subjets above some E_T threshold within this LRJ
    int nSubjets {0};

    // Number of input objects within this LRJ
    int nConstituents {0};

    // Indices (into the constituent input container passed to makeJets) of
    // every input object merged into this LRJ
    std::vector<int> constituentsIndices;

    // Indices (into the seed-source container passed at construction) of every
    // input object merged into this LRJ
    std::vector<int> mergedIndices;

    // Per-subjet kinematics for substructure studies / ntupling.
    std::vector<float> subjet_et;
    std::vector<float> subjet_eta;
    std::vector<float> subjet_phi;

    // Substructure observables (undigitised), zero if not computed.
    float psi_R {0};
    float tau_1 {0};
    float tau_2 {0};
    float tau_21 {0};
    float massApprox {0};
  };

}

#endif // TRIGGEPPERF_LARGERJET_H
