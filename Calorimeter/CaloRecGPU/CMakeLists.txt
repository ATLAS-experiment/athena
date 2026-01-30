# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( CaloRecGPU )

# The build of this package needs CUDA. If it's not available, don't try to do
# anything...
if( NOT CMAKE_CUDA_COMPILER )
  message( STATUS "CUDA not found, CaloRecGPU is not built" )
  return()
endif()

find_package( Boost COMPONENTS chrono unit_test_framework)

# Add a component library that has some CUDA code in it.
atlas_add_library( CaloRecGPULib
   CaloRecGPU/*.h src/*.cxx src/*.cu src/*.h
   PUBLIC_HEADERS CaloRecGPU
   LINK_LIBRARIES AthenaBaseComps AthenaInterprocess CaloEvent CaloInterfaceLib CxxUtils GaudiKernel TileEvent TrigT2CaloCommonLib xAODTrigCalo
   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )

# Suppress spurious warnings seen in the LTO build originating 
# in the boost parser code.
set_target_properties( CaloRecGPULib
                       PROPERTIES LINK_FLAGS " -Wno-maybe-uninitialized "  )

atlas_add_component( CaloRecGPU
   src/components/*.cxx
   LINK_LIBRARIES CaloRecGPULib )
   
atlas_add_test( CaloRecUtilities_test
   SOURCES
   test/CaloRecUtilities_test.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES CaloRecGPULib ${Boost_LIBRARIES}
   POST_EXEC_SCRIPT nopost.sh
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

set_target_properties( CaloRecGPULib PROPERTIES
   CUDA_SEPARABLE_COMPILATION ON
   POSITION_INDEPENDENT_CODE ON
   INTERFACE_POSITION_INDEPENDENT_CODE ON
   CUDA_STANDARD 17
   CUDA_STANDARD_REQUIRED TRUE )

target_compile_definitions(CaloRecGPULib PRIVATE CUDA_API_PER_THREAD_DEFAULT_STREAM=1)
target_compile_options(CaloRecGPULib PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-expt-relaxed-constexpr>)
