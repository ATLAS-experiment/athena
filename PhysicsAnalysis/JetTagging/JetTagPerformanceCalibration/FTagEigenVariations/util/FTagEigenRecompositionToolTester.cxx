/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagEigenVariations/IFTagEigenRecompositionTool.h"

#include "AsgTools/StandaloneToolHandle.h"
#include "CxxUtils/checker_macros.h"
#ifndef XAOD_STANDALONE
#include "POOLRootAccess/TEvent.h"
#endif

#include <cmath>
#include <format>
#include <string>
#include <vector>

ANA_MSG_HEADER(testRecomposition)
ANA_MSG_SOURCE(testRecomposition, "FTagEigenRecompositionToolTester")
using namespace testRecomposition;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {
  if (argc < 6) {
    ANA_MSG_ERROR("Usage: <exe> <CDI file> <tagger> <jet collection> <operating point> <SFEigen|SFGlobalEigen>");
    return 1;
  }
  const std::string strategy = argv[5];
#ifndef XAOD_STANDALONE
  POOL::Init();
#endif

  asg::StandaloneToolHandle<ftag::ev::IFTagEigenRecompositionTool> tool(
    "ftag::ev::FTagEigenRecompositionTool/RecompositionToolTester");
  if (tool.setProperty("ScaleFactorFileName", std::string(argv[1])).isFailure() ||
      tool.setProperty("TaggerName", std::string(argv[2])).isFailure() ||
      tool.setProperty("JetAuthor", std::string(argv[3])).isFailure() ||
      tool.setProperty("OperatingPoint", std::string(argv[4])).isFailure() ||
      tool.setProperty("SystematicsStrategy", strategy).isFailure() || tool.initialize().isFailure()) {
    ANA_MSG_ERROR("Failed to set up the tool");
    return 1;
  }

  for (const std::string label : {"B", "C", "T", "Light"}) {
    const ftag::ev::RecompositionTable& table = tool->recomposition(label);
    ANA_MSG_INFO(std::format("{}: {} nuisance parameters, {} sources", label, table.nuisanceParameters.size(),
                             table.sources.size()));
    for (std::size_t k = 0; k < table.nuisanceParameters.size(); ++k) {
      // Leading source of each nuisance parameter
      std::size_t lead = 0;
      for (std::size_t s = 1; s < table.sources.size(); ++s) {
        if (std::abs(table.coefficients[k][s]) > std::abs(table.coefficients[k][lead])) lead = s;
      }
      ANA_MSG_INFO(std::format("  {:<28} described {:.3f}, leading {} ({:+.3f})", table.nuisanceParameters[k],
                               table.describedFraction[k], table.sources.empty() ? "-" : table.sources[lead],
                               table.sources.empty() ? 0.0 : table.coefficients[k][lead]));
    }
  }
  return 0;
}
