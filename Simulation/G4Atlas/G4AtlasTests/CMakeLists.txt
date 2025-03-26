# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( G4AtlasTests )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( G4AtlasTests
                   src/*.cxx src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}  ${GEANT4_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${CLHEP_LIBRARIES} AtlasHepMCLib ${GEANT4_LIBRARIES} AthenaBaseComps GaudiKernel CaloDetDescrLib CaloIdentifier CaloSimEvent AthenaKernel StoreGateLib GeoAdaptors GeoPrimitives Identifier xAODEventInfo ALFA_SimEv LUCID_SimEvent ZDC_SimEvent ZdcIdentifier GeneratorObjects InDetSimEvent LArSimEvent MuonReadoutGeometry MuonIdHelpersLib MuonSimEvent G4AtlasToolsLib MCTruth HitManagement TileDetDescr TileIdentifier TileSimEvent TrackRecordLib TRT_ReadoutGeometry )
set_target_properties( G4AtlasTests PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/sim_*.py )

