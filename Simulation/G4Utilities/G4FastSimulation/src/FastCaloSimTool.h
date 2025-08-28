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

/* Random generator service include */
#include "AthenaKernel/IAthRNGSvc.h"

class G4VFastSimulationModel;

class FastCaloSimTool: public FastSimulationBase
{
 public:

  FastCaloSimTool(const std::string& type, const std::string& name, const IInterface *parent);   //!< Default constructor
  ~FastCaloSimTool() {}

  /** Begin of an athena event - do any thing that needs to be done at the beginning of each *athena* event. **/
  virtual StatusCode BeginOfAthenaEvent() override final;
  /** End of an athena event - do any thing that needs to be done at the end of each *athena* event. **/
  virtual StatusCode EndOfAthenaEvent() override final;

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

  // Random generator service
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "RandomSvc", "AthRNGSvc", ""};
  // Random generator engine name
  Gaudi::Property<std::string> m_randomEngineName{this, "RandomStream", ""};
  // Name of associated CaloCellContainerSD
  Gaudi::Property<std::string> m_CaloCellContainerSDName{this, "CaloCellContainerSDName", "", "Name of the associated CaloCellContainerSD"};
  // Flag to enable G4 transportation
  Gaudi::Property<bool> m_doG4Transport{this, "doG4Transport", false, "Flag to enable G4 transportation"};
  /// @brief Optional flags that allow to further limit the usage of fast simulation beyond the default configuration
  /// Note: For fast simulation jobs, default values should be used. They are only useful in special cases
  /// where you want to further limit the usage of fast sim, e.g. if we want to replace certain parts of full simulation
  Gaudi::Property<bool>  m_doPhotons{this, "doPhotons", true, "Flag to enable FCS simulation for photons"};
  Gaudi::Property<bool>  m_doElectrons{this, "doElectrons", true, "Flag to enable FCS simulation for electrons and positrons"};
  Gaudi::Property<bool>  m_doHadrons{this, "doHadrons", true, "Flag to enable FCS simulation for pions and other hadrons"};
  Gaudi::Property<float> m_AbsEtaMin{this, "AbsEtaMin", 0, "Abs(Eta) lower bound for FastCaloSim"};
  Gaudi::Property<float> m_AbsEtaMax{this, "AbsEtaMax", 10, "Abs(Eta) upper bound for FastCaloSim"};
  Gaudi::Property<float> m_EkinMinPhotons{this, "EkinMinPhotons", 0, "Kinetic photon energy lower bound for FastCaloSim"};
  Gaudi::Property<float> m_EkinMaxPhotons{this, "EkinMaxPhotons", std::numeric_limits<float>::max(), "Kinetic photon energy upper bound for FastCaloSim"};
  Gaudi::Property<float> m_EkinMinElectrons{this, "EkinMinElectrons", 0, "Kinetic electron energy lower bound for FastCaloSim"};
  Gaudi::Property<float> m_EkinMaxElectrons{this, "EkinMaxElectrons", std::numeric_limits<float>::max(), "Kinetic electron energy upper bound for FastCaloSim"};
  Gaudi::Property<bool>  m_doPunchThrough{this, "doPunchThrough", true, "Run punchthrough simulation for particle entering Calo-MS boundary"};
};

#endif //G4FASTSIMULATION_FASTCALOSIMTOOL_H
