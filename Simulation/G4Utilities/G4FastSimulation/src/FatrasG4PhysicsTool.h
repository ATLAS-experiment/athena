/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4PHYSICSTOOL_H
#define G4FASTSIMULATION_FATRASG4PHYSICSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

#include <string>
#include <vector>

/** Switches off Geant4 electromagnetic processes everywhere inside the
 EnvelopeVolumes, in every region with a root volume inside them, for every
 particle, by giving each a zero cross section model there:
 every one of them with DisableAllG4Physics, otherwise those named in
 InactiveProcesses. Geant4 can do this for
 G4VEmProcess, G4VEnergyLossProcess
 and G4VMultipleScattering only, so the hadronic processes, decay and the TRT
 transition radiation (XTR) stay on. **/
class FatrasG4PhysicsTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  FatrasG4PhysicsTool(const std::string& type, const std::string& name,
                      const IInterface* parent);

  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       const std::vector<std::string>& envelopeVolumes,
                       bool disableAll, const std::vector<std::string>& inactiveProcesses);

    void ConstructParticle() override;
    void ConstructProcess() override;

   private:
    std::vector<std::string> m_envelopeVolumes;
    bool m_disableAll;
    std::vector<std::string> m_inactiveProcesses;
  };

 private:
  Gaudi::Property<std::vector<std::string>> m_envelopeVolumes{this, "EnvelopeVolumes", {}, "Volumes where FatrasG4 runs, everything inside them included"};
  Gaudi::Property<bool> m_disableAll{this, "DisableAllG4Physics", false, "Switch off every Geant4 electromagnetic process in the regions"};
  Gaudi::Property<std::vector<std::string>> m_inactiveProcesses{this, "InactiveProcesses", {}, "Geant4 electromagnetic processes switched off in the regions"};
};

#endif
