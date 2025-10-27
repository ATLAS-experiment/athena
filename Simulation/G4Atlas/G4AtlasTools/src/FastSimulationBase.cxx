/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlasTools/FastSimulationBase.h"

// Geant4 includes used in functions
#include "G4AutoDelete.hh"
#include <G4Region.hh>
#include "G4RegionStore.hh"

FastSimulationBase::FastSimulationBase(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

G4Region* FastSimulationBase::getRegion() const
{
  if (m_regionName.value().empty()) {
    return nullptr;
  }
  return G4RegionStore::GetInstance()->GetRegion(m_regionName.value());
}


// Athena method, used to get out the G4 geometry and set up the Fast Simulation Models
StatusCode FastSimulationBase::initializeFastSim(){
  ATH_MSG_VERBOSE( name() << "::initializeFastSim()" );

  // Instantiate the FastSimModel for this geant4 thread and register it for deletion
  G4AutoDelete::Register(makeFastSimModel());

  return StatusCode::SUCCESS;
}
