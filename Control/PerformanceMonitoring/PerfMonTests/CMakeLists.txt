# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PerfMonTests )

# External dependencies:
find_package( CLHEP )

# Component(s) in the package:
atlas_add_component( PerfMonTests
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} AthenaBaseComps AthenaKernel AthAllocators AthContainers CxxUtils GaudiKernel )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test ( PerfMonMTSvc_serial
  SCRIPT test/test_perfMonMTSvc_serial.py
  POST_EXEC_SCRIPT "nopost.sh"
  PROPERTIES TIMEOUT 300)

atlas_add_test ( PerfMonMTSvc_mt1
  SCRIPT test/test_perfMonMTSvc_mt1.py
  POST_EXEC_SCRIPT "nopost.sh"
  PROPERTIES TIMEOUT 300)

atlas_add_test ( PerfMonMTSvc_mt8
  SCRIPT test/test_perfMonMTSvc_mt8.py
  POST_EXEC_SCRIPT "nopost.sh"
  PROPERTIES TIMEOUT 300)
