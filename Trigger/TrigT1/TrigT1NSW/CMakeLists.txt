# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1NSW )

# External dependencies:
find_package( ROOT COMPONENTS Tree )

# Component(s) in the package:
atlas_add_component( TrigT1NSW
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps CxxUtils GaudiKernel MuonRDO TrigT1NSWSimToolsLib )

