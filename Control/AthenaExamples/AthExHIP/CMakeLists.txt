# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set up the package name.
atlas_subdir( AthExHIP )

# Check if HIP compilation is available.
if( NOT CMAKE_HIP_COMPILER )
   message( STATUS "HIP not available, not building package" )
   return()
endif()

# External dependencies.
find_package( traccc )

# Set up the build of the example library.
atlas_add_component( AthExHIP
   src/*.h src/*.cxx src/*.hip src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps AthenaInterprocess xAODCore xAODTracking
                  AthDeviceInterfacesLib AthHIPInterfacesLib
                  StoreGateLib GaudiKernel traccc::core )
set_target_properties( AthExHIP PROPERTIES
   LINKER_LANGUAGE HIP )

# Install extra files from the package.
atlas_install_python_modules( python/*.py )

# Test(s) in the package.
atlas_add_test( LinearTransformExample
   SCRIPT athena.py --threads=1 AthExHIP/LinearTransformExampleConfig.py
   POST_EXEC_SCRIPT noerror.sh)
