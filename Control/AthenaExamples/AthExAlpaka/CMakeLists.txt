# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( AthExAlpaka )

# External dependencies.
option( alpaka_ACC_CPU_B_SEQ_T_THREADS_ENABLE
   "Enable the threads CPU block thread back-end" ON )
find_package( alpaka )

# Add a component library that has pure Alpaka code in it.
atlas_add_component( AthExAlpaka
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES alpaka::alpaka
   AthenaBaseComps GaudiKernel )

# Install extra files from the package.
atlas_install_python_modules( python/*.py )

# Test(s) in the package.
atlas_add_test( UnitTest
   SCRIPT athena.py --threads=1 AthExAlpaka/AthExAlpakaConfig.py
   POST_EXEC_SCRIPT noerror.sh)
