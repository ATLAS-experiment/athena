/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// test interface for a wrapper module that can open various kinds of neural
// network graph file

// include things to test
#include "ISF_FastCaloSimEvent/TFCSEnergyAndHitGANV2.h"
#include "ISF_FastCaloSimEvent/TFCSPredictExtrapWeights.h"

// include generic utilities
#include "ISF_FastCaloSimEvent/MLogging.h"
#include <fstream>
#include <iostream>
#include <vector>

using ISF_FCS::MLogging;
//coverity[UNCAUGHT_EXCEPT]
int main() {
  ISF_FCS::MLogging logger;
  ATH_MSG_NOCLASS(logger, "Running TFCSEnergyAndHitGANV2 on LWTNN");
  std::string path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastCaloSim/LWTNNsample";
  TFCSEnergyAndHitGANV2::test_path(path, nullptr, nullptr, nullptr, "unnamed", 211);

  // This causes timeouts in CI
  // ATH_MSG_NOCLASS(logger, "Running TFCSEnergyAndHitGANV2 on ONNX");
  // path = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/FastCaloSim/ONNXsample";
  // TFCSEnergyAndHitGANV2::test_path(path, nullptr, nullptr, nullptr, "unnamed", 211);
  ATH_MSG_NOCLASS(logger, "Program ends");
  return 0;
}
