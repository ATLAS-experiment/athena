/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AtlasPhysListFactory_h
#define AtlasPhysListFactory_h 1

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

#ifdef ATHSIMULATION_USE_G4HEPEM
#include "G4PhysicsLists/G4HepEmOptions.h"
#endif

class AtlasPhysListFactory
{
public:

  AtlasPhysListFactory();

  ~AtlasPhysListFactory()=default;

  G4VModularPhysicsList* GetReferencePhysList(const G4String&);
  // instantiate PhysList by name

  G4VModularPhysicsList* ReferencePhysList();
  // instantiate PhysList by environment variable "PHYSLIST"

  G4bool IsReferencePhysList(const G4String&);
  // check if the name is in the list of PhysLists names

  const std::vector<G4String>& AvailablePhysLists() const;
  // list of avalable base Phys Lists

  inline void SetVerbose(G4int val) { m_verbose = val; }
#ifdef ATHSIMULATION_USE_G4HEPEM
  inline void SetHepEmOptions(const G4HepEmOptions& val) { m_hepEmOptions = val; }
#endif

private:

  G4String m_defName;  
  G4int m_verbose;
#ifdef ATHSIMULATION_USE_G4HEPEM
  G4HepEmOptions m_hepEmOptions{};
#endif
  std::vector<G4String> m_listnames;
};

#endif
