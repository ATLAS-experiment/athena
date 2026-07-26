/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FatrasG4Tool.h"
#include "FatrasG4.h"


FatrasG4Tool::FatrasG4Tool(const std::string& type, const std::string& name, const IInterface *parent)
: FastSimulationBase(type, name, parent)
{
}

G4VFastSimulationModel* FatrasG4Tool::makeFastSimModel()
{
  ATH_MSG_DEBUG("Initializing Fast Simulation Model FatrasG4");

  // Create the FatrasG4 fast simulation model
  return new FatrasG4(name(), getRegion(), this);
}
