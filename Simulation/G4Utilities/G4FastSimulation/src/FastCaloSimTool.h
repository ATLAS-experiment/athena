/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_FASTCALOSIMTOOL_H
#define G4FASTSIMULATION_FASTCALOSIMTOOL_H

/* Fast simulation base include */
#include "G4AtlasTools/FastSimulationBase.h"
/* FastCaloSim parametrization service include */
#include "ISF_FastCaloSimInterfaces/IFastCaloSimParamSvc.h"
/* FastCaloSim calorimeter transportation include */
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloTransportation.h"
/* FastCaloSim calorimeter extrapolation include */
#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloExtrapolation.h"
/* Geant4 transportation tool */
#include "G4AtlasInterfaces/IG4CaloTransportTool.h"
// Geant4 Punchthrough G4 Tool
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"

class G4VFastSimulationModel;

class FastCaloSimTool: public FastSimulationBase
{
 public:

  FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent);   //!< Default constructor

protected:
  /** Method to make the actual fast simulation model itself, which
   will be owned by the tool.  Must be implemented in all concrete
   base classes. */
  virtual G4VFastSimulationModel* makeFastSimModel() override final;  
 
 private:
  
  // FastCaloSim service 
  ServiceHandle<ISF::IFastCaloSimParamSvc> m_FastCaloSimSvc{this, "ISF_FastCaloSimV2ParamSvc", "ISF_FastCaloSimV2ParamSvc"};
  // FastCaloSim transportation tool
  PublicToolHandle<IFastCaloSimCaloTransportation> m_FastCaloSimCaloTransportation{this, "FastCaloSimCaloTransportation", "FastCaloSimCaloTransportation", ""};
  // FastCaloSim extrapolation tool
  PublicToolHandle<IFastCaloSimCaloExtrapolation> m_FastCaloSimCaloExtrapolation{this, "FastCaloSimCaloExtrapolation", "FastCaloSimCaloExtrapolation", ""};
  // Geant4 transportation tool
  PublicToolHandle<IG4CaloTransportTool> m_G4CaloTransportTool{this, "G4CaloTransportTool", "G4CaloTransportTool", ""};
  // Geant4 Punchthrough G4 Tool
  PublicToolHandle<IPunchThroughSimWrapper> m_PunchThroughSimWrapper{this, "PunchThroughSimWrapper", "PunchThroughSimWrapper", ""};

  // Name of associated CaloCellContainerSD
  Gaudi::Property<std::string> m_CaloCellContainerSDName{this, "CaloCellContainerSDName", "", "Name of the associated CaloCellContainerSD"};
  // Flag to enable G4 transportation
  Gaudi::Property<bool> m_doG4Transport{this, "doG4Transport", false, "Flag to enable G4 transportation"};
  // Flag to enable punch-through simulation
  Gaudi::Property<bool>  m_doPunchThrough{this, "doPunchThrough", true, "Run punchthrough simulation for particle entering Calo-MS boundary"};
};

#endif //G4FASTSIMULATION_FASTCALOSIMTOOL_H
