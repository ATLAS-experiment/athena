/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FATRASG4TOOL_H
#define G4FASTSIMULATION_FATRASG4TOOL_H

/* Fast simulation base include */
#include "G4AtlasTools/FastSimulationBase.h"

/* Geant4 transportation tool */
#include "G4AtlasInterfaces/IG4FatrasTransportTool.h"

class G4VFastSimulationModel;

class FatrasG4Tool: public FastSimulationBase
{
 public:

  FatrasG4Tool(const std::string& type, const std::string& name, const IInterface *parent);   //!< Default constructor

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;  
 
 private:
  
  // Flag to enable G4 transportation
  Gaudi::Property<bool> m_doG4Transport{this, "doG4Transport", false, "Flag to enable G4 transportation"};

  // Geant4 transportation tool
  PublicToolHandle<IG4FatrasTransportTool> m_G4FatrasTransportTool{this, "G4FatrasTransportTool", "G4FatrasTransportTool", ""};
};

#endif //G4FASTSIMULATION_FATRASG4TOOL_H
