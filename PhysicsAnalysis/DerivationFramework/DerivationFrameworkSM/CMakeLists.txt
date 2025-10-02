# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkSM )


# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core MathCore )
find_package( HepPDT )
# Component(s) in the package:
atlas_add_component( DerivationFrameworkSM
   DerivationFrameworkSM/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${HEPPDT_LIBRARIES} ${ROOT_LIBRARIES}
   DerivationFrameworkInterfaces GaudiKernel StoreGateLib xAODTracking TrigDecisionToolLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
