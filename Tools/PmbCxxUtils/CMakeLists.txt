# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: PmbCxxUtils
################################################################################

# Find TBB
find_package( TBB )

# Declare the package name:
atlas_subdir( PmbCxxUtils )

# Component(s) in the package:
atlas_add_library( PmbCxxUtils
                   src/*.cxx
                   PUBLIC_HEADERS PmbCxxUtils
                   INCLUDE_DIRS ${TBB_INCLUDE_DIRS}
                   LINK_LIBRARIES ${TBB_LIBRARIES} )

# Test(s) in the package:
atlas_add_test( BasicStopWatch_test
                SOURCES test/BasicStopWatch_test.cxx
                LINK_LIBRARIES PmbCxxUtils
                POST_EXEC_SCRIPT noerror.sh )
