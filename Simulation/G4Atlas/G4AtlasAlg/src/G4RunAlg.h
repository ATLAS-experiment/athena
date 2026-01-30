/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4RunAlg_H
#define G4ATLASALG_G4RunAlg_H

// Base class header
#include "AthenaBaseComps/AthAlgorithm.h"

// STL headers
#include <map>
#include <string>

// Gaudi headers
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ToolHandle.h"

// Athena headers
#include "CxxUtils/checker_macros.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "G4AtlasInterfaces/IFastSimulationMasterTool.h"
#include "G4AtlasInterfaces/IG4RunTool.h"
#include "G4AtlasInterfaces/ISensitiveDetectorMasterTool.h"
#include "G4AtlasInterfaces/IUserActionSvc.h"
#include "GeneratorObjects/McEventCollection.h"
#include "HepMC_Interfaces/IZeroLifetimePatcher.h"
#include "xAODEventInfo/EventInfo.h"

// ISF includes
#include "ISF_Interfaces/ITruthSvc.h"
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Interfaces/IInputConverter.h"
#include "ISF_Interfaces/IGenEventFilter.h"

/// @class G4RunAlg
/// @brief Primary Athena algorithm for ATLAS simulation.
///
/// This algorithm setup a Geant Run (through G4RunTool) which executes in a separate thread
/// pool managed by the G4RunManager. 
///
/// In contrast to G4AtlasAlg, in which execute() runs the actual transport loop of the event, G4RunAlg
/// will only prepare the event, and push it to the G4 worker using a shared queue. Condition variables 
/// are used to synchronize the Athena worker thread with the G4 worker thread in execute. 
///
class G4RunAlg : public AthAlgorithm
{
public:

  /// Standard algorithm constructor
  using AthAlgorithm::AthAlgorithm;

  /// this Alg is Clonable (for AthenaMT)
  virtual bool isClonable() const override { return true; }

  /// @brief Initialize the algorithm.
  ///
  /// Here we setup several things for simulation, including:
  /// - force intialization of the UserActionSvc
  /// - apply custom G4 UI commands (like custom physics list)
  /// - configure the particle generator and random generator svc
  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;

  /// @brief Simulate one Athena event.
  virtual StatusCode execute() override;

private:
  /// Releases the GeoModel geometry from memory once it has been used
  /// to build the G4 geometry and is no-longer required
  void releaseGeoModel();

  /// @name Configurable Properties
  /// @{
  Gaudi::Property<bool> m_flagAbortedEvents{this, "FlagAbortedEvents", false, ""};
  Gaudi::Property<bool> m_killAbortedEvents{this, "KillAbortedEvents", false, ""};
  Gaudi::Property<bool> m_releaseGeoModel{this, "ReleaseGeoModel", true, ""};
  Gaudi::Property<bool> m_useShadowEvent{this, "UseShadowEvent", false, "New approach to selecting particles for simulation"};
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName", "Geant4", ""};
  Gaudi::Property<std::string> m_simplifiedGeoPath{this, "SimplifiedGeoPath", "", "Path to the simplified geometry file"};
  Gaudi::Property<std::map<std::string, std::string>> m_verbosities {this, "Verbosities", {}, "Map of G4 Verbosities to set for the simulation"};
  
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfo", "EventInfo", "EventInfo key"};
  SG::ReadHandleKey<McEventCollection> m_inputTruthCollectionKey{this, "InputTruthCollection", "BeamTruthEvent", "Input hard scatter collection"};
  SG::WriteHandleKey<McEventCollection> m_outputTruthCollectionKey{this, "OutputTruthCollection", "TruthEvent", "Output hard scatter truth collection"};

  /// Geo ID Service
  ServiceHandle<ISF::IGeoIDSvc> m_geoIDSvc{this, "GeoIDSvc", "ISF_GeoIDSvc", ""};
  /// Service to convert ISF_Particles into a G4Event
  ServiceHandle<ISF::IInputConverter> m_inputConverter{this, "InputConverter", "ISF_InputConverter", ""};
  /// Central Truth Service
  ServiceHandle<ISF::ITruthSvc> m_truthRecordSvc{this, "TruthRecordService", "ISF_TruthRecordSvc", ""};
  /// Quasi-Stable Particle Simulation Patcher
  ServiceHandle<Simulation::IZeroLifetimePatcher> m_qspatcher{this, "QuasiStablePatcher", "", "Quasi-Stable Particle Simulation Patcher"};
  /// Random number service
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "AtRndmGenSvc", "AthRNGSvc", ""};
  /// User Action Service
  ServiceHandle<G4UA::IUserActionSvc> m_userActionSvc{this, "UserActionSvc", "G4UA::UserActionSvc", ""};

  /// Fast Simulation Master Tool
  PublicToolHandle<IFastSimulationMasterTool> m_fastSimTool{this, "FastSimMasterTool", "FastSimulationMasterTool", ""};
  /// G4Atlas Tool for thread management and data interface
  PublicToolHandle<IG4RunTool> m_g4RunTool{this, "G4RunTool", "G4RunTool", ""};
  /// Sensitive Detector Master Tool
  PublicToolHandle<ISensitiveDetectorMasterTool> m_senDetTool{this, "SenDetMasterTool", "SensitiveDetectorMasterTool", ""};
  /// Tool for filtering out quasi-stable particle daughters
  ToolHandle<ISF::IGenEventFilter> m_truthPreselectionTool{this, "TruthPreselectionTool", "", "Tool for filtering out quasi-stable particle daughters"};
  /// @}
};

#endif// G4ATLASALG_G4RunAlg_H
