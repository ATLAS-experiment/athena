//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "xAODDataSource/MakeDataFrame.h"

#include "RDataSource.h"

// System include(s).
#include <memory>

namespace xAOD {

ROOT::RDataFrame MakeDataFrame(std::string_view fileNameGlob,
                               std::string_view treeName, bool verboseOutput,
                               TEvent::EAuxMode auxmode) {

  auto source = std::make_unique<RDataSource>(fileNameGlob, treeName);
  source->setVerboseOutput(verboseOutput);
  source->setAuxMode(auxmode);
  return ROOT::RDataFrame(std::move(source));
}

ROOT::RDataFrame MakeDataFrame(const std::vector<std::string>& fileNames,
                               std::string_view treeName, bool verboseOutput,
                               TEvent::EAuxMode auxmode) {

  auto source = std::make_unique<RDataSource>(fileNames, treeName);
  source->setVerboseOutput(verboseOutput);
  source->setAuxMode(auxmode);
  return ROOT::RDataFrame(std::move(source));
}

}  // namespace xAOD
