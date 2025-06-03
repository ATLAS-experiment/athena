/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_GEANT4TOOLS_G4LEGACYTRANSPORTTOOL_H
#define ISF_GEANT4TOOLS_G4LEGACYTRANSPORTTOOL_H



// Gaudi headers
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// DetectorDescription
#include "AtlasDetDescr/AtlasRegion.h"

// Athena headers
#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaKernel/SlotSpecificObj.h"
#include "G4AtlasInterfaces/IUserActionSvc.h"
#include "G4AtlasInterfaces/IDetectorGeometrySvc.h"
#include "G4AtlasInterfaces/ISensitiveDetectorMasterTool.h"
#include "G4AtlasInterfaces/IFastSimulationMasterTool.h"
#include "G4AtlasInterfaces/IPhysicsListSvc.h"
#include "G4AtlasInterfaces/IUserLimitsSvc.h"
#include "CxxUtils/checker_macros.h"

// ISF includes
#include "ISF_Geant4Tools/IG4RunManagerHelper.h"
#include "ISF_Interfaces/BaseSimulatorG4Tool.h"
#include "ISF_Interfaces/IInputConverter.h"
#include "ISF_Interfaces/ISimulationSelector.h"
// STL headers
#include <memory>
#include <string>
#include <unordered_map>

class G4Event;
class G4Timer;
class G4AtlasRunManager;

namespace ISF {
  class ISFParticle;
}

#include "AtlasHepMC/GenEvent_fwd.h"

namespace iGeant4
{

  /** @class G4LegacyTransportTool

      Geant4 AlgTool to create a ISFParticle at a volume entry/exit
      - universal transport tool

      @author Robert Harrington
  */

class ATLAS_NOT_THREAD_SAFE G4LegacyTransportTool
    : public ISF::BaseSimulatorG4Tool {

 public:
  /** Constructor */
  G4LegacyTransportTool(const std::string&, const std::string&,
                        const IInterface*);

  /** Destructor */
  virtual ~G4LegacyTransportTool() = default;

  /** AlgTool initialize method */
  virtual StatusCode initialize() override final;

  /// G4 initialization called only by the first tool instance.
  /// This is done (for now) because we get multiple tool instances in hive.
  void initializeOnce ATLAS_NOT_THREAD_SAFE();

  /** AlgTool finalize method */
  virtual StatusCode finalize() override final;

  /// G4 finalization called only by the first tool instance.
  /// This is done (for now) because we get multiple tool instances in hive.
  void finalizeOnce();

  using ISF::BaseSimulatorG4Tool::releaseEvent;
  using ISF::BaseSimulatorG4Tool::setupEvent;
  using ISF::BaseSimulatorG4Tool::simulate;
  using ISF::BaseSimulatorG4Tool::simulateVector;

  virtual StatusCode simulate(const EventContext& ctx, ISF::ISFParticle& isp,
                              ISF::ISFParticleContainer& secondaries,
                              McEventCollection* mcEventCollection,
                              std::shared_ptr<HitCollectionMap>) override;

  virtual StatusCode simulateVector(
      const EventContext& ctx, const ISF::ISFParticleVector& particles,
      ISF::ISFParticleContainer& secondaries,
      McEventCollection* mcEventCollection, std::shared_ptr<HitCollectionMap> hitCollections,
      McEventCollection* shadowTruth = nullptr) override;

  virtual StatusCode setupEvent(const EventContext&,
                                HitCollectionMap&) override;

  virtual StatusCode releaseEvent(const EventContext&,
                                  HitCollectionMap&) override;

  virtual ISF::SimulationFlavor simFlavor() const override {
    return ISF::Geant4;
  };

 private:
  /// This command prints a message about a G4Command depending on its
  /// returnCode
  void commandLog(int returnCode, const std::string& commandString) const;

  HepMC::GenEvent* genEvent(McEventCollection* mcEventCollection) const;

  /// @name Configurable Properties
  /// @{

  // timing checks
  Gaudi::Property<bool> m_doTiming{this, "PrintTimingInfo", true, ""};
  // float m_runTime;
  float m_accumulatedEventTime{0.};
  float m_accumulatedEventTimeSq{0.};
  unsigned int m_nrOfEntries{0};

  G4Timer* m_runTimer{nullptr};
  G4Timer* m_eventTimer{nullptr};

  // store secondary particles that have been pushed back
  struct Slot {
    std::unordered_map<ISF::ISFParticle const*, ISF::ISFParticleContainer>
        m_secondariesMap;
    typedef std::mutex mutex_t;
    typedef std::lock_guard<mutex_t> lock_t;
    mutex_t m_mutex;
  };
  mutable SG::SlotSpecificObj<Slot> m_slots ATLAS_THREAD_SAFE;
  Gaudi::Property<std::string> m_mcEventCollectionName{
      this, "McEventCollection", "TruthEvent", ""};
  /// Helper Tool to provide G4RunManager
  PublicToolHandle<ISF::IG4RunManagerHelper> m_g4RunManagerHelper{
      this, "G4RunManagerHelper",
      "iGeant4::G4RunManagerHelper/G4RunManagerHelper", ""};
  G4AtlasRunManager* m_pRunMgr{};

  Gaudi::Property<std::string> m_libList{this, "Dll", "", ""};
  Gaudi::Property<std::string> m_physList{this, "Physics", "", ""};
  Gaudi::Property<std::string> m_fieldMap{this, "FieldMap", "", ""};
  Gaudi::Property<bool> m_releaseGeoModel{this, "ReleaseGeoModel", true, ""};
  Gaudi::Property<bool> m_recordFlux{this, "RecordFlux", false, ""};
  Gaudi::Property<bool> m_quietMode{this, "QuietMode", false, ""};
  /// Commands to send to the G4 UI
  Gaudi::Property<std::vector<std::string> > m_g4commands{
      this, "G4Commands", {}, "Commands to send to the G4UI"};
  /// Activate multi-threading configuration
  Gaudi::Property<bool> m_useMT{this, "MultiThreading", false,
                                "Multi-threading specific settings"};
  Gaudi::Property<bool> m_activateParallelGeometries{
      this, "ActivateParallelWorlds", false,
      "Toggle on/off the G4 parallel geometry system"};
  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmGenSvc{this, "RandomNumberService",
                                         "AthRNGSvc", ""};
  /// Random Stream Name
  Gaudi::Property<std::string> m_randomStreamName{this, "RandomStreamName",
                                                  "Geant4", ""};
  ///
  ServiceHandle<IUserLimitsSvc> m_userLimitsSvc{this, "UserLimitsSvc",
                                                "UserLimitsSvc", ""};
  /// user action service
  ServiceHandle<G4UA::IUserActionSvc> m_userActionSvc{this, "UserActionSvc", "",
                                                      ""};
  /// Detector Geometry Service (builds G4 Geometry)
  ServiceHandle<IDetectorGeometrySvc> m_detGeoSvc{this, "DetGeoSvc",
                                                  "DetectorGeometrySvc", ""};
  /// Service to convert ISF_Particles into a G4Event
  ServiceHandle<ISF::IInputConverter> m_inputConverter{
      this, "InputConverter", "ISF_InputConverter", ""};
  /// Physics List Service
  ServiceHandle<IPhysicsListSvc> m_physListSvc{this, "PhysicsListSvc",
                                               "PhysicsListSvc", ""};
  /// Sensitive Detector Master Tool
  PublicToolHandle<ISensitiveDetectorMasterTool> m_senDetTool{
      this, "SenDetMasterTool", "SensitiveDetectorMasterTool", ""};
  /// Fast Simulation Master Tool
  PublicToolHandle<IFastSimulationMasterTool> m_fastSimTool{
      this, "FastSimMasterTool", "FastSimulationMasterTool", ""};
  /// @}
};
}


#endif // ISF_GEANT4TOOLS_G4LEGACYTRANSPORTTOOL_H
