/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Include files
#include <type_traits>

#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Version.hh"

#include "PathResolver/PathResolver.h"

// local
#include "G4AtlasTools/G4AtlasDetectorConstructionTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : G4AtlasDetectorConstructionTool
//
// 2014-10-03: Andrea Dell'Acqua
//-----------------------------------------------------------------------------


//=================================
// Standard constructor, initializes variables
//=================================
G4AtlasDetectorConstructionTool::G4AtlasDetectorConstructionTool( const std::string& type,
                                                                  const std::string& nam,const IInterface* parent )
  : base_class( type, nam , parent )
{
}

//=================================
// Athena method overrides
//=================================
StatusCode G4AtlasDetectorConstructionTool::initialize( )
{
  // Resolve the file early, but let Geant4 import it in Construct().
  if (!m_simplifiedGeoPath.value().empty()) {
    m_simplifiedGeoFile = PathResolverFindCalibFile(m_simplifiedGeoPath.value());
    if (m_simplifiedGeoFile.empty()) {
      ATH_MSG_FATAL("Could not find simplified geometry file: " << m_simplifiedGeoPath.value());
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_DEBUG( "Initializing Geometry configuration tools "  );
  for (auto it: m_configurationTools)
  {
    ATH_CHECK( it.retrieve() );
    ATH_CHECK( it->preGeometryConfigure() );
  }

  ATH_MSG_DEBUG( "Initializing World detectors in " << name() );
  ATH_CHECK( m_detTool.retrieve() );
  ATH_CHECK( m_notifierSvc.retrieve() );

  ATH_MSG_DEBUG( "Initializing sensitive detectors in " << name() );
  ATH_CHECK( m_senDetTool.retrieve() );

  ATH_MSG_DEBUG( "Initializing fastsim in " << name() );
  ATH_CHECK( m_fastSimTool.retrieve() );

  ATH_MSG_DEBUG( "Setting up G4 physics regions" );
  for (auto& it: m_regionCreators)
  {
    ATH_CHECK( it.retrieve() );
  }

  if (m_activateParallelWorlds)
  {
    ATH_MSG_DEBUG( "Setting up G4 parallel worlds" );
    for (auto& it: m_parallelWorlds)
    {
      ATH_CHECK( it.retrieve() );
    }
  }

  ATH_MSG_DEBUG( "Setting up field managers" );
  ATH_CHECK( m_fieldManagers.retrieve() );

  return StatusCode::SUCCESS;
}

std::vector<std::string>& G4AtlasDetectorConstructionTool::GetParallelWorldNames()
{
  return m_parallelWorldNames;
}

auto G4AtlasDetectorConstructionTool::GetDetectorConstruction()
    -> UPDetectorConstruction {
  static_assert(std::has_virtual_destructor_v<G4VUserDetectorConstruction>,
                "G4VUserDetectorConstruction must have a virtual destructor");
  return {
      new G4AtlasDetectorConstruction(this),
      [](G4VUserDetectorConstruction* ptr) { delete ptr; }};
}

//=================================
// G4VUserDetectorConstruction method overrides
//=================================
G4AtlasDetectorConstructionTool::G4AtlasDetectorConstruction::
    ~G4AtlasDetectorConstruction() {
  // The master run manager deletes its detector construction after joining
  // all workers and before deleting the Geant4 kernel. Clean the shared
  // physical volumes in that safe teardown window, including when run-manager
  // destruction is caused by an early return or exception.
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
}

G4VPhysicalVolume*
G4AtlasDetectorConstructionTool::G4AtlasDetectorConstruction::Construct() {
  // Import the FastCaloSim transport world on the Geant4 master thread, before
  // workers create their fast-simulation models.
  if (!m_detConstructionTool->m_simplifiedGeoFile.empty()) {
    auto* logicalVolumeStore = G4LogicalVolumeStore::GetInstance();
    if (!logicalVolumeStore->GetVolume("WorldLog", false)) {
      ATH_MSG_INFO("Reading simplified transport geometry from "
                   << m_detConstructionTool->m_simplifiedGeoFile);
      // GDML references require the imported volumes to keep their file names.
      const bool namePrefixing =
          m_detConstructionTool->m_notifierSvc->GetNamePrefixing();
      m_detConstructionTool->m_notifierSvc->SetNamePrefixing(false);
      G4GDMLParser parser;
      try {
        parser.Read(m_detConstructionTool->m_simplifiedGeoFile, false);
      } catch (...) {
        m_detConstructionTool->m_notifierSvc->SetNamePrefixing(namePrefixing);
        throw;
      }
      m_detConstructionTool->m_notifierSvc->SetNamePrefixing(namePrefixing);
    }
  }

  ATH_MSG_DEBUG("Detectors " << m_detConstructionTool->m_detTool.name()
                             << " being set as World");
  m_detConstructionTool->m_detTool->SetAsWorld();
  m_detConstructionTool->m_detTool->Build();

  ATH_MSG_DEBUG( "Setting up G4 physics regions" );
  for (auto& it : m_detConstructionTool->m_regionCreators) {
    it->Construct();
  }

  if (m_detConstructionTool->m_activateParallelWorlds) {
    ATH_MSG_DEBUG( "Setting up G4 parallel worlds" );
    for (auto& it : m_detConstructionTool->m_parallelWorlds) {
      m_detConstructionTool->m_parallelWorldNames.push_back(it.name());
      this->RegisterParallelWorld(it->GetParallelWorld());
    }
  }

  ATH_MSG_DEBUG( "Running geometry post-configuration tools" );
  for (auto it : m_detConstructionTool->m_configurationTools) {
    StatusCode sc = it->postGeometryConfigure();
    if (!sc.isSuccess())
    {
      ATH_MSG_FATAL( "Unable to run post-geometry configuration for " << it->name() );
    }
  }

  // Build world volume and rebuild LV/PV stores if Geant4 is 11 or newer
  // - Rebuild necessary because Athena may install LV/PV notifiers that change
  //   volume names, which invalidates store maps.
  G4VPhysicalVolume* wv = m_detConstructionTool->m_detTool->GetWorldVolume();
#if G4VERSION_NUMBER > 1079
  G4LogicalVolumeStore::GetInstance()->SetMapValid(false);
  G4LogicalVolumeStore::GetInstance()->UpdateMap();
  G4PhysicalVolumeStore::GetInstance()->SetMapValid(false);
  G4PhysicalVolumeStore::GetInstance()->UpdateMap();
#endif

  return wv;
}

void G4AtlasDetectorConstructionTool::G4AtlasDetectorConstruction::
    ConstructSDandField() {
  ATH_MSG_DEBUG( "Setting up sensitive detectors" );
  if (m_detConstructionTool->m_senDetTool->initializeSDs().isFailure()) {
    ATH_MSG_FATAL("Failed to initialize SDs for worker thread");
  }

  if(!m_detConstructionTool->m_fastSimTool->initializeFastSims().isSuccess()) {
    ATH_MSG_FATAL("Failed to initialize Fast Simulation Tool for worker thread");
    return;
  }

  ATH_MSG_DEBUG( "Setting up field managers" );
  for (auto& fm : m_detConstructionTool->m_fieldManagers) {
    StatusCode sc = fm->initializeField();
    if (!sc.isSuccess())
    {
      ATH_MSG_FATAL( "Unable to initialise field with " << fm->name() );
      return;
    }
  }

  return;
}
