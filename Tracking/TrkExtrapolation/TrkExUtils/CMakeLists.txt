# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkExUtils )

find_package( GTest )

# Component(s) in the package:
atlas_add_library( TrkExUtils
                   src/*.cxx
                   PUBLIC_HEADERS TrkExUtils
                   LINK_LIBRARIES GeoPrimitives EventPrimitives GaudiKernel TrkDetDescrUtils TrkGeometry
		           TrkSurfaces TrkEventPrimitives TrkMaterialOnTrack TrkNeutralParameters TrkParameters CxxUtils
                   TrkPatternParameters )

# Some functions here make heavy use of Eigen and are thus very slow in debug
# builds.  Set up to allow forcing them to compile with optimization and
# inlining, even in debug builds.
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" )
  set_source_files_properties(
     ${CMAKE_CURRENT_SOURCE_DIR}/src/RungeKuttaUtils.cxx
     PROPERTIES
     COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )
endif()

atlas_install_python_modules( python/*.py)

atlas_add_test( MaterialInteraction_pdg_comparison
    SCRIPT python -m TrkExUtils.pdg_comparison
)

atlas_add_test( MaterialInteraction
   SOURCES test/MaterialInteractionTest.cxx
   LINK_LIBRARIES
   TrkExUtils
   TrkGeometry
   TrkEventPrimitives
   GTest::gtest_main
   POST_EXEC_SCRIPT noerror.sh
)


