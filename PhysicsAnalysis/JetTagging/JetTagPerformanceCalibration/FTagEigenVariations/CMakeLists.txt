# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FTagEigenVariations )

# External dependencies:
find_package( Eigen )
find_package( GTest )

# Component(s) in the package:
atlas_add_library( FTagEigenVariationsLib
                   Root/*.cxx
                   PUBLIC_HEADERS FTagEigenVariations
                   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${EIGEN_LIBRARIES} )

# Test(s) in the package:
atlas_add_test( gt_EigenDecomposition
                SOURCES test/gt_EigenDecomposition.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} CxxUtils FTagEigenVariationsLib
                POST_EXEC_SCRIPT noerror.sh )
