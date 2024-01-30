# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZDC_SimuDigitization )

# External dependencies:
find_package( ROOT )
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( ZDC_SimuDigitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel PileUpToolsLib ZdcUtilsLib ZDC_SimEvent ZdcEvent xAODForward GaudiKernel HitManagement ZdcIdentifier)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

