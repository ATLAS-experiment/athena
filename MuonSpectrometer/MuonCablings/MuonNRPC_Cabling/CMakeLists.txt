# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonNRPC_Cabling )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
atlas_add_component( MuonNRPC_Cabling
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib MuonIdHelpersLib 
                                    MuonCablingData AthenaPoolUtilities PathResolver SGTools )

atlas_install_python_modules( python/*.py 
                             POST_BUILD_CMD ${ATLAS_FLAKE8} )