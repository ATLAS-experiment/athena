/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "G4AtlasTools/FastSimulationBase.h"
#include <G4Region.hh>

// Geant4 includes used in functions
#include "G4RegionStore.hh"
#include "G4FastSimulationManager.hh"

FastSimulationBase::FastSimulationBase(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

FastSimulationBase::~FastSimulationBase()
{
  deleteFastSimModel();
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

  // Make sure Fast Simulation Model isn't already registered
  if(getFastSimModel()){
    ATH_MSG_ERROR("Trying to create a Fast Simulation Model which already exists!");
    return StatusCode::FAILURE;
  }

  // Make the FastSimModel stored by this tool
  auto* fastsimmodel = makeFastSimModel();
  setFastSimModel(fastsimmodel);

  // Set the verbosity information on this thing - this will have to go into the makeFastSimModel methods...
  //if(msgLvl(MSG::VERBOSE)) m_FastSimModel->SetVerboseLevel(10);
  //else if(msgLvl(MSG::DEBUG)) m_FastSimModel->SetVerboseLevel(5);

  return StatusCode::SUCCESS;
}

G4VFastSimulationModel* FastSimulationBase::getFastSimModel()
{
#ifdef G4MULTITHREADED
  // Get current thread-ID
  const auto tid = std::this_thread::get_id();
  // Retrieve it from the FastSimModel map
  auto fastsimmodelPair = m_fastsimmodelThreadMap.find(tid);
  if(fastsimmodelPair == m_fastsimmodelThreadMap.end()) return nullptr;
  return fastsimmodelPair->second;
#else
  return m_FastSimModel;
#endif
}

void FastSimulationBase::setFastSimModel(G4VFastSimulationModel* fastsimmodel)
{
#ifdef G4MULTITHREADED
  // Make sure one isn't already assigned
  const auto tid = std::this_thread::get_id();
  ATH_MSG_DEBUG("Creating and registering FastSimModel " << fastsimmodel << " in thread " << tid);
  m_fastsimmodelThreadMap.insert( std::make_pair(tid, fastsimmodel) );
#else
  m_FastSimModel = fastsimmodel;
#endif
}

void FastSimulationBase::deleteFastSimModel()
{
#ifdef G4MULTITHREADED
  for(auto& threadMapPair : m_fastsimmodelThreadMap)
    {
      auto fastSimModel = threadMapPair.second;
      if (fastSimModel)
	delete fastSimModel;      
    }
  m_fastsimmodelThreadMap.clear();
#else
  if(m_FastSimModel)
    {
      delete m_FastSimModel;
      m_FastSimModel = 0;
    }
#endif
}
