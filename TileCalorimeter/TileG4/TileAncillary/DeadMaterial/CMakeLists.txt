# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: DeadMaterial
################################################################################

# Declare the package name:
atlas_subdir( DeadMaterial )

# External dependencies:
find_package( CLHEP )
find_package( Geant4 )
find_package( GeoModel COMPONENTS GeoModelKernel )

# Component(s) in the package:
atlas_add_library( DeadMaterial
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${GEOMODEL_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${GEANT4_LIBRARIES} ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} StoreGateLib GaudiKernel G4AtlasToolsLib GeoMaterial2G4 GeoModelInterfaces )
set_target_properties( DeadMaterial PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_runtime( share/deadmaterial.dtd share/deadmaterial.xml )

