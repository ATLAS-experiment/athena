# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AFP_G4_SD )

# External dependencies:
find_package( Geant4 )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( AFP_G4_SDLib
                   src/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${GEANT4_INCLUDE_DIRS}
                   LINK_LIBRARIES ${GEANT4_LIBRARIES} ${GTEST_LIBRARIES} AFP_Geometry AFP_SimEv G4AtlasToolsLib StoreGateLib )
set_target_properties( AFP_G4_SDLib PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

atlas_add_library( AFP_G4_SD
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_LINK_LIBRARIES AFP_G4_SDLib )
set_target_properties( AFP_G4_SD PROPERTIES INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/optionForTest.txt )

