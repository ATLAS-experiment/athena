################################################################################
# Package: TrigGpuTest
################################################################################

# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

link_libraries(stdc++fs)

# Set the name of the package.
atlas_subdir( TrigGpuTest )

# External dependencies:

find_package(TBB)

atlas_install_scripts( test/test*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_executable( trigGpuTest
   src/*.cxx
   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
   LINK_LIBRARIES TrigAccelEvent ${TBB_LIBRARIES} rt dl)

# Unit tests:
atlas_add_test( TrigValSteeringUT
                SCRIPT trigvalsteering-unit-tester.py ${CMAKE_CURRENT_SOURCE_DIR}/test
                PROPERTIES TIMEOUT 300
                POST_EXEC_SCRIPT noerror.sh )
