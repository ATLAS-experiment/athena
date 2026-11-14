# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FTagEigenVariations )

# External dependencies:
find_package( Eigen )
find_package( GTest )
find_package( ROOT COMPONENTS Core Hist Matrix RIO )

# Component(s) in the package:
atlas_add_library( FTagEigenVariationsLib
                   Root/*.cxx
                   PUBLIC_HEADERS FTagEigenVariations
                   INCLUDE_DIRS ${EIGEN_INCLUDE_DIRS}
                   LINK_LIBRARIES ${EIGEN_LIBRARIES} )

atlas_add_library( FTagEigenVariationsCDILib
                   Root/cdi/*.cxx
                   PUBLIC_HEADERS FTagEigenVariations
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} FTagEigenVariationsLib CalibrationDataInterfaceLib
                   PRIVATE_LINK_LIBRARIES CxxUtils )

# Test(s) in the package:
atlas_add_test( gt_EigenDecomposition
                SOURCES test/gt_EigenDecomposition.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} CxxUtils FTagEigenVariationsLib
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( gt_EigenReductionRecomposition
                SOURCES test/gt_EigenReductionRecomposition.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} CxxUtils FTagEigenVariationsLib
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( gt_CDIInput
                SOURCES test/gt_CDIInput.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils FTagEigenVariationsCDILib
                POST_EXEC_SCRIPT noerror.sh )

# Comparison with the current implementation in CalibrationDataInterface, to be removed together with it.
# The label also runs it in the CI of merge requests that change CalibrationDataInterface.
atlas_add_test( gt_CompareCurrentImplementation
                SOURCES test/gt_CompareCurrentImplementation.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} ${ROOT_LIBRARIES} CxxUtils FTagEigenVariationsCDILib PathResolver
                LABELS CalibrationDataInterface
                POST_EXEC_SCRIPT noerror.sh )
