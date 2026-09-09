# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( EFTrackingDataTransfer )

# Make use of the SYCL support in VecMem for building this package.
find_package( vecmem COMPONENTS LANGUAGE )

# Check if SYCL is available as a language.
vecmem_check_language( SYCL )
if( NOT CMAKE_SYCL_COMPILER )
   message( STATUS "SYCL is not available, not building EFTrackingDataTransfer" )
   return()
endif()

# Enable SYCL as a language.
enable_language( SYCL )

# Tests
atlas_add_test( TransferBenchmark_test
	SOURCES test/TransferBenchmark_test.cxx
	LINK_LIBRARIES TestTools ActsEventLib xAODInDetMeasurement
	POST_EXEC_SCRIPT noerror.sh)

atlas_add_test( SyclTransferBenchmark_test 
	SOURCES test/SyclTransferBenchmark_test.sycl
	LINK_LIBRARIES TestTools ActsEventLib xAODInDetMeasurement AsgTools
	POST_EXEC_SCRIPT noerror.sh)

