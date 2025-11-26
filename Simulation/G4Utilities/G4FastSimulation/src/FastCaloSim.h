/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FASTCALOSIM_H
#define G4FASTSIMULATION_FASTCALOSIM_H

// Service handle to define smart pointer to FastCaloSim parametrisation service
#include "GaudiKernel/ServiceHandle.h"
// Geant4 fast simulation base class
#include "G4VFastSimulationModel.hh"
// FastCaloSim parametrisation service interface
#include "ISF_FastCaloSimInterfaces/IFastCaloSimParamSvc.h"
// FastCaloSim extrapolation tool interface
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloExtrapolation.h"
// FastCaloSim ATLAS transportation tool interface
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloTransportation.h"
// Geant4 transportation tool interface
#include "G4AtlasInterfaces/IG4CaloTransportTool.h"
// Geant4 Punchthrough G4 Tool
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"

// FastCaloSim tool
#include "FastCaloSimTool.h"


class CaloCellContainerSD;
class G4FieldTrack;
class G4SafetyHelper;

class FastCaloSim: public G4VFastSimulationModel
{
 public:

  FastCaloSim(const std::string& name,
              G4Region* region,
              const PublicToolHandle<IFastCaloSimCaloTransportation>& FastCaloSimCaloTransportation,
              const PublicToolHandle<IFastCaloSimCaloExtrapolation>& FastCaloSimCaloExtrapolation,
              const PublicToolHandle<IG4CaloTransportTool>& G4CaloTransportTool,
              const PublicToolHandle<IPunchThroughSimWrapper>& PunchThroughSimWrapper,
              const ServiceHandle<ISF::IFastCaloSimParamSvc>& FastCaloSimSvc,
              const std::string& CaloCellContainerSDName,
              bool doG4Transport,
              bool doPunchThrough,
              FastCaloSimTool * FastCaloSimTool);
  ~FastCaloSim() {}

  G4bool IsApplicable(const G4ParticleDefinition&) override final;
  void DoIt(const G4FastTrack&, G4FastStep&) override final;

  /** Determines the applicability of the fast sim model to this particular track.
  Checks that geometric location, energy, and particle type are within bounds.  Also checks for
  containment of the particle's shower within a specific detector region. **/
  G4bool ModelTrigger(const G4FastTrack &) override final;

  /** Retrieves the associated sensitive detector responsible for writing out the CaloCellContainer **/
  CaloCellContainerSD * getCaloCellContainerSD();

  /** Check if the particle is located at the proper ID-Calo parametrization boundary and is travelling outwards from the ID to the CALO **/
  G4bool passedIDCaloBoundary(const G4FastTrack& fastTrack);
  
 private:
  // FastCaloSimCaloTransportation tool to transport particles through the detector with the ATLAS tracking tools 
  PublicToolHandle<IFastCaloSimCaloTransportation> m_FastCaloSimCaloTransportation;
  // FastCaloSimCaloExtrapolation tool to extrapolate particle shower positions to layers
  PublicToolHandle<IFastCaloSimCaloExtrapolation> m_FastCaloSimCaloExtrapolation;
  // Geant4 transportation tool
  PublicToolHandle<IG4CaloTransportTool> m_G4CaloTransportTool;
  // Geant4 Punchthrough G4 Tool
  PublicToolHandle<IPunchThroughSimWrapper> m_PunchThroughSimWrapper;

  // Main FastCaloSim service
  ServiceHandle<ISF::IFastCaloSimParamSvc> m_FastCaloSimSvc;
  // Name of associated CaloCellContainer sensitive detector
  std::string m_CaloCellContainerSDName;
  // Boolean flag to enable Geant4 transportation
  bool m_doG4Transport;

  //For PunchThrough
  bool m_doPunchThrough;
};

#endif //G4FASTSIMULATION_FASTCALOSIM_H

