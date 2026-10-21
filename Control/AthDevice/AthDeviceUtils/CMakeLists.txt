# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthDeviceUtils)

# External dependencies.
find_package(vecmem)
find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package.
atlas_add_library(AthDeviceUtilsLib
   AthDeviceUtils/*.h
   INTERFACE
   PUBLIC_HEADERS AthDeviceUtils
   LINK_LIBRARIES GaudiKernel AthContainers AthDeviceInterfacesLib vecmem::core)

# Test(s) in the package.
atlas_add_test(simple_vector_test
   SOURCES test/simple_vector_test.cxx
           test/HostMRAndCpFixture.*
           test/VecMemTypes.h
           test/xAODTypes.*
   LINK_LIBRARIES AthDeviceUtilsLib xAODCore TestTools
                  Boost::unit_test_framework
   ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share"
   POST_EXEC_SCRIPT noerror.sh)
