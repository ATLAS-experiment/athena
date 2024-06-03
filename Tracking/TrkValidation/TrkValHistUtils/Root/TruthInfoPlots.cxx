/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkValHistUtils/TruthInfoPlots.h"
#include "AthContainers/ConstAccessor.h"

namespace Trk {
  void
  TruthInfoPlots::init() {
    truthType = nullptr;
    origin = nullptr;
  }

  void
  TruthInfoPlots::initializePlots() {
    truthType = Book1D("truthType", "truthType;truthType;Entries", 50, -0.5, 49.5);
    origin = Book1D("truthOrigin", "truthOrigin;truthOrigin;Entries", 40, -0.5, 39.5);
  }

  void
  TruthInfoPlots::fill(const xAOD::TruthParticle &truthprt, float weight ) {
    static const SG::ConstAccessor<int> truthTypeAcc ("truthType");
    if (truthTypeAcc.isAvailable (truthprt)) {
      truthType->Fill(truthTypeAcc (truthprt), weight);
    }
    static const SG::ConstAccessor<int> truthOriginAcc ("truthOrigin");
    if (truthOriginAcc.isAvailable (truthprt)) {
      origin->Fill(truthOriginAcc (truthprt), weight);
    }
  }
}
