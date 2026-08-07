/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_JET_H
#define TRIGL0GEPPERF_JET_H

#include "TLorentzVector.h"

namespace Gep{
  struct Jet
  {
   
    TLorentzVector vec;
    // Raw eta/phi as delivered by the seed source, before the TLorentzVector
    // (SetPtEtaPhiM) round-trip. JetTaggerLRJ digitizes these to match the
    // emulation, which reads the same values as written (float) to the ntuple.
    double etaInput {0};
    double phiInput {0};
    std::vector<int> constituentsIndices;
    int nConstituents {0};
    float radius {0};
    float seedEta {0}; // Only for Seeded jets
    float seedPhi {0};
    float seedEt {0};
    float ring0_Et {0}; // Only for WTACone4jets
    float ring1_Et {0};
    float ring2_Et {0};
    float ring3_Et {0};
    float ring4_Et {0};
    int total_TobN {0};
    int ring0_TobN {0};
    int ring1_TobN {0};
    int ring2_TobN {0};
    int ring3_TobN {0};
    int ring4_TobN {0};
    
  };
}

#endif //TRIGL0GEPPERF_CUSTOMJET_H
