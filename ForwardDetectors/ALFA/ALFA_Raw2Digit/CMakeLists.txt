# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ALFA_Raw2Digit )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Gpad )

# Component(s) in the package:
atlas_add_component( ALFA_Raw2Digit
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps ALFA_RawEv GaudiKernel PathResolver )

# Install files from the package:
atlas_install_data( mapping/*.dat )
