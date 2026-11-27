# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_G4_SD )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GeoModelCore )
find_package( GeoModelIO )

# Component(s) in the package:
atlas_add_library( HGTD_G4_SD
                   src/*.h src/*.cxx src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} 
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEANT4_LIBRARIES} GeoModelCore::GeoModelKernel GeoModelIO::GeoModelRead 
                                    GeoModelIO::GeoModelDBManager StoreGateLib HGTD_Identifier HitManagement InDetSimEvent 
                                    G4AtlasToolsLib MCTruth GeoModelInterfaces GeoPrimitives )
set_target_properties( HGTD_G4_SD PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py )
