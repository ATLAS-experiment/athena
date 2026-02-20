// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#include "GeneratorInfoSvc.h"

GeneratorInfoSvc::GeneratorInfoSvc(const std::string& name, ISvcLocator* svcLoc)
  : AthService(name, svcLoc)
{
}

StatusCode GeneratorInfoSvc::initialize() {
  return StatusCode::SUCCESS;
}
