/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_AFATRASG4TOOL_H
#define G4FASTSIMULATION_AFATRASG4TOOL_H

#include "HitManagement/HitCollectionMap.h"

/* Fast simulation base include */
#include "G4AtlasTools/FastSimulationBase.h"

/* Geant4 ACTSFatras G4 Tool */
#include "G4AtlasInterfaces/IActsFatrasG4Tool.h"

// Hits writing
#include "StoreGate/WriteHandleKey.h"
#include "InDetSimEvent/SiHitCollection.h"
#include "InDetSimEvent/SiHit.h"

class G4VFastSimulationModel;
class AFatrasG4Tool: public FastSimulationBase
{
public:

  using FastSimulationBase::FastSimulationBase;

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */

  // ==============================
  // Exposed FastSimulation lifecycle hooks
  // ==============================
  virtual StatusCode initializeFastSim() override final;
  virtual StatusCode BeginOfAthenaEvent(HitCollectionMap&) override final;
  virtual StatusCode EndOfAthenaEvent(HitCollectionMap&) override final;

  virtual G4VFastSimulationModel* makeFastSimModel() override final;  
 
 private:

  // Geant4 ACTSFatras G4 Tool
  PublicToolHandle<IActsFatrasG4Tool> m_ActsFatrasG4Tool{this, "ActsFatrasG4Tool", "ActsFatrasG4Tool", ""};

  // ==============================
  // Write handles owned by this tool
  // ==============================
  SG::WriteHandleKey<SiHitCollection> m_pixelHitsKey{
      this,
      "PixelCollectionName",
      "PixelHits_ActsFatrasG4",
      "StoreGate key for Pixel SiHitCollection"
  };

  SG::WriteHandleKey<SiHitCollection> m_sctHitsKey{
      this,
      "SCTCollectionName",
      "SCT_Hits_ActsFatrasG4",
      "StoreGate key for SCT SiHitCollection"
  };  
};

#endif //G4FASTSIMULATION_AFATRASG4TOOL_H
