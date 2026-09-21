/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_NOG4PHYSICSTOOL_H
#define G4FASTSIMULATION_NOG4PHYSICSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

#include <string>
#include <vector>

/** Switches off the Geant4 electromagnetic physics in the given regions, for
 every particle and at every energy, by giving each process a zero cross
 section model there. The fast simulation owns those regions, so Geant4 is left
 with the transport alone and anything the fast models do not cover crosses
 them without interacting.

 Only the electromagnetic processes can be replaced per region; the hadronic
 ones and decay have no such mechanism in Geant4 and stay on. **/
class NoG4PhysicsTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  NoG4PhysicsTool(const std::string& type, const std::string& name,
                  const IInterface* parent);

  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       const std::vector<std::string>& regionNames);

    void ConstructParticle() override;
    void ConstructProcess() override;

   private:
    std::vector<std::string> m_regionNames;
  };

 private:
  Gaudi::Property<std::vector<std::string>> m_regionNames{this, "RegionNames", {}, "Regions the fast simulation owns, where Geant4 physics is switched off"};
};

#endif
