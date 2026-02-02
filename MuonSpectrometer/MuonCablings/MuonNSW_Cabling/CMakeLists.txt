# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonNSW_Cabling )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
atlas_add_component( MuonNSW_Cabling
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib MuonIdHelpersLib 
                                    MuonCablingData AthenaPoolUtilities PathResolver SGTools )


# Install files from the package:
atlas_install_python_modules( python/*.py )