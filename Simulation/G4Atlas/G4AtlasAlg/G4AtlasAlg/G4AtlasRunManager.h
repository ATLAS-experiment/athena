/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4ATLASALG_G4AtlasRunManager_h
#define G4ATLASALG_G4AtlasRunManager_h

// Base class header
#include "G4RunManager.hh"

// Gaudi headers
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"

// Athena headers
#include "AthenaBaseComps/AthMessaging.h"
#include "CxxUtils/checker_macros.h"
#include "G4AtlasInterfaces/IDetectorConstructionTool.h"
#include "G4AtlasInterfaces/IPhysicsListSvc.h"
#include "G4AtlasInterfaces/IFluxRecorder.h"

/// ATLAS custom singleton run manager.
///
/// This is the run manager used for serial (not-MT) jobs.
/// @todo sync and reduce code duplication with MT run managers.
///

class G4AtlasRunManager: public G4RunManager, public AthMessaging {

public:

  virtual ~G4AtlasRunManager() {}

  /// Retrieve the singleton instance
  static G4AtlasRunManager* GetG4AtlasRunManager ATLAS_NOT_THREAD_SAFE ();

  /// Does the work of simulating an ATLAS event
  bool ProcessEvent(G4Event* event);

  /// G4 function called at start of run
  void RunInitialization() override final;

  /// G4 function called at end of run
  void RunTermination() override final;

  /// Configure the detector construction tool
  void SetDetConstructionTool(IDetectorConstructionTool* detConstruction) {
    m_detConstruction = detConstruction;
  }

  /// Configure the Physics List Tool handle
  void SetPhysListSvc(const std::string& typeAndName) {
    m_physListSvc.setTypeAndName(typeAndName);
  }

  void SetRecordFlux(bool b, std::unique_ptr<IFluxRecorder> f) { m_recordFlux = b; m_fluxRecorder=std::move(f);}
  void SetLogLevel(int) { /* Not implemented */ }

  void SetVolumeSmartlessLevel(const std::map<std::string,double>& nameAndValue){
    m_volumeSmartlessLevel = nameAndValue;
  }

  /// Configure the QuietMode option
  void SetQuietMode(bool quietMode) {
    m_quietMode = quietMode;
  }

  /// Bring in all overloads from G4RunManager
  using G4RunManager::SetUserInitialization;

  /// Allow user worker initialization for single-threaded runmanager
  void SetUserInitialization(G4UserWorkerInitialization* userInit) override {
    userWorkerInitialization = userInit;
  }

protected:

  /// @name Overridden G4 init methods for customization
  /// @{
  void Initialize() override final;
  void InitializeGeometry() override final;
  void InitializePhysics() override final;
  /// @}

private:

  /// Pure singleton private constructor
  G4AtlasRunManager();

  void EndEvent();

  bool m_recordFlux;

  ServiceHandle<IPhysicsListSvc> m_physListSvc;

  IDetectorConstructionTool* m_detConstruction{nullptr};

  /// Interface to flux recording

  std::unique_ptr<IFluxRecorder> m_fluxRecorder;

  //Property to allow an arbitrary volume (named by string) to have its
  //"smartless" value set
  std::map<std::string, double> m_volumeSmartlessLevel;

  /// Quiet Mode for production
  bool m_quietMode{true};

};

#endif // G4ATLASALG_G4AtlasRunManager_h
