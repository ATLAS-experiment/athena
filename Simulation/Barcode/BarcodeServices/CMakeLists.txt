# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BarcodeServices )

# External dependencies:
find_package( TBB )

# Component(s) in the package:
atlas_add_component( BarcodeServices
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
                     LINK_LIBRARIES ${TBB_LIBRARIES} AthenaBaseComps BarcodeInterfacesLib GaudiKernel TruthUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

