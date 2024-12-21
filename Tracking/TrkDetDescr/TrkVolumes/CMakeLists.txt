# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkVolumes )
find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_library( TrkVolumes
                   src/*.cxx
                   PUBLIC_HEADERS TrkVolumes
		           LINK_LIBRARIES AthenaKernel GeoPrimitives GaudiKernel TrkDetDescrUtils TrkGeometrySurfaces TrkSurfaces TrkEventPrimitives TrkParameters CxxUtils)


                         # Code in this file makes heavy use of eigen and runs orders of magnitude
# more slowly without optimization.  So force this to be optimized even
# in debug builds.  If you need to debug it you might want to change this.
# Specifying optimization via an attribute on the particular
# function didn't work, because that still didn't allow inlining.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/Volume.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()

atlas_add_test(test_VolumeTests
           SOURCES test/test_VolumeTests.cxx
           INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
           LINK_LIBRARIES ${Boost_LIBRARIES} TrkVolumes TrkSurfaces
           POST_EXEC_SCRIPT nopost.sh)
