/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4PHYSICSTOOL_H
#define G4FASTSIMULATION_FATRASG4PHYSICSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPhysicsOptionTool.h"
#include "G4AtlasInterfaces/IPhysicsConstructor.h"

#include "GaudiKernel/SystemOfUnits.h"

#include <string>
#include <vector>

/** Switches off Geant4 gamma conversion where the FatrasG4 ACTS trigger is
 valid, in its regions and energy window, by giving the conversion process a
 zero cross section model there. Outside the window the regions get the model
 Geant4 converts with everywhere else, so those photons still convert. **/
class FatrasG4PhysicsTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  FatrasG4PhysicsTool(const std::string& type, const std::string& name,
                      const IInterface* parent);

  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       const std::vector<std::string>& regionNames,
                       double minEnergy, double maxEnergy);

    void ConstructParticle() override;
    void ConstructProcess() override;

   private:
    std::vector<std::string> m_regionNames;
    double m_minEnergy;
    double m_maxEnergy;
  };

 private:
  Gaudi::Property<std::vector<std::string>> m_regionNames{this, "RegionNames", {}, "Regions where FatrasG4 is valid"};
  Gaudi::Property<double> m_minEnergy{this, "MinEnergy", 1.*Gaudi::Units::GeV, "Lower end of the FatrasG4 energy window"};
  Gaudi::Property<double> m_maxEnergy{this, "MaxEnergy", 100.*Gaudi::Units::GeV, "Upper end of the FatrasG4 energy window"};
};

/** Removes every Geant4 photon process but gamma conversion, so that photons
 are only transported and converted. **/
class GammaConversionOnlyPhysicsTool final : public extends<AthAlgTool, IPhysicsOptionTool> {
 public:
  GammaConversionOnlyPhysicsTool(const std::string& type, const std::string& name,
                                 const IInterface* parent);

  UPPhysicsConstructor GetPhysicsOption() override;

  class PhysicsConstructor : public IPhysicsContructor {
   public:
    PhysicsConstructor(const std::string& name, MSG::Level level,
                       const std::vector<std::string>& processNames);

    void ConstructParticle() override;
    void ConstructProcess() override;

   private:
    std::vector<std::string> m_processNames;
  };

 private:
  Gaudi::Property<std::vector<std::string>> m_processNames{this, "ProcessNames",
      {"phot", "compt", "Rayl", "photonNuclear", "GammaToMuPair"}, "Geant4 photon processes removed"};
};

#endif
