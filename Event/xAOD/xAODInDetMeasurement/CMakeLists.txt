# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODInDetMeasurement )

# External dependencies:
find_package( xAODUtilities )
find_package( benchmark )

# Component(s) in the package.
atlas_add_library( xAODInDetMeasurement
    xAODInDetMeasurement/*.h xAODInDetMeasurement/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODInDetMeasurement
    LINK_LIBRARIES xAODCore GeoPrimitives xAODMeasurementBase Identifier )

atlas_add_xaod_smart_pointer_dicts(
    INPUT xAODInDetMeasurement/selection.xml
    OUTPUT _selectionFile
    CONTAINERS
       "xAOD::PixelClusterContainer_v1"
       "xAOD::HGTDClusterContainer_v1"
       "xAOD::StripClusterContainer_v1"
       "xAOD::SpacePointContainer_v1" )

atlas_add_dictionary( xAODInDetMeasurementDict
    xAODInDetMeasurement/xAODInDetMeasurementDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODInDetMeasurement
    EXTRA_FILES Root/dict/*.cxx )

# Test(s) in the package.
foreach( _testname PixelCluster HGTDCluster StripCluster SpacePoint )
   atlas_add_test( xAODInDetMeasurement_${_testname}_test
      SOURCES "test/xAODInDetMeasurement_${_testname}_test.cxx"
      LINK_LIBRARIES xAODInDetMeasurement )
endforeach()

# Benchmark(s) in the package.
atlas_add_executable( xAODInDetMeasurement_benchmarks
    "test/PixelCluster_benchmark.cxx"
    LINK_LIBRARIES AthAllocators xAODInDetMeasurement
                   benchmark::benchmark benchmark::benchmark_main )
