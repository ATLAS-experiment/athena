#Copyright(C) 2002 - 2025 CERN for the benefit of the ATLAS collaboration

#Declare the package name:
atlas_subdir( TrkGaussianSumFilterUtils )
find_package( Boost COMPONENTS container)
#Component(s) in the package:
atlas_add_library( TrkGaussianSumFilterUtilsLib
                   src/*.cxx
		   PUBLIC_HEADERS TrkGaussianSumFilterUtils
		   LINK_LIBRARIES EventPrimitives TrkParameters 
		                  TrkEventPrimitives TrkMeasurementBase
                      TrkGeometry PathResolver
           PRIVATE_LINK_LIBRARIES CxxUtils PathResolver TrkExUtils
           PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
if( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" )
  # Suppress useless warning about changed ABI for pair<> on arm.
  set_target_properties( TrkGaussianSumFilterUtilsLib PROPERTIES COMPILE_FLAGS " -Wno-psabi " )
endif()

#Tests
atlas_add_test(ut_GSF_testAlignedDynArray
    SOURCES test/testAlignedDynArray.cxx
    LINK_LIBRARIES TrkGaussianSumFilterUtilsLib)

atlas_add_test(ut_GSF_testMergeComponents
    SOURCES test/testMergeComponents.cxx
    LINK_LIBRARIES CxxUtils TrkGaussianSumFilterUtilsLib)

atlas_add_test(ut_GSF_testFindMode
    SOURCES test/testFindMode.cxx
    LINK_LIBRARIES CxxUtils TrkGaussianSumFilterUtilsLib)

atlas_add_test(ut_GSF_testHorner
    SOURCES test/testHorner.cxx
	LINK_LIBRARIES CxxUtils TrkGaussianSumFilterUtilsLib PathResolver TrkExUtils)

atlas_add_test(ut_GSF_findIndexOfMinimum
    SOURCES test/testFindIndexOfMinimum.cxx
	LINK_LIBRARIES CxxUtils TrkGaussianSumFilterUtilsLib)

