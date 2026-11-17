# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkPixelByteStreamCnv )

set(ATLAS_ITKSW_ENDEC_SOURCE_DIR "" CACHE PATH "Optional local source director for Endec")

if(IS_DIRECTORY "${ATLAS_ITKSW_ENDEC_SOURCE_DIR}")
  message(STATUS "Using Endec from: ${ATLAS_ITKSW_ENDEC_SOURCE_DIR}")

  set(ENDEC_BUILD_TESTS FALSE CACHE BOOL "Turn off the endec unit tests")
  set(ENDEC_BUILD_PYTHON_BINDINGS FALSE CACHE BOOL "Turn off endec python bindings")

  include(FetchContent)
  set(FETCHCONTENT_SOURCE_DIR_ITKSW_ENDEC "${ATLAS_ITKSW_ENDEC_SOURCE_DIR}" CACHE PATH "")

  FetchContent_Declare(itksw_endec
    SYSTEM
    OVERRIDE_FIND_PACKAGE
  )
endif()

# External dependencies:
find_package( tdaq-common COMPONENTS eformat )
find_package( Boost COMPONENTS unit_test_framework )
find_package( itksw_endec )

# Component(s) in the package:
atlas_add_library( ITkPixelByteStreamCnvLib
   ITkPixelByteStreamCnv/*.h
   INTERFACE
   PUBLIC_HEADERS ITkPixelByteStreamCnv
   INCLUDE_DIRS src
   LINK_LIBRARIES GaudiKernel PathResolver ByteStreamData InDetByteStreamErrors InDetRawData )

atlas_add_component( ITkPixelByteStreamCnv
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${TDAQ-COMMON_INCLUDE_DIRS}
   LINK_LIBRARIES ${TDAQ-COMMON_LIBRARIES} itksw::pix_endec ByteStreamData AthenaKernel EventContainers
   GaudiKernel InDetRawData AthenaBaseComps AthContainers CxxUtils StoreGateLib
   ByteStreamCnvSvcBaseLib InDetIdentifier ITkPixelCablingLib PixelReadoutGeometryLib IRegionSelector
   xAODEventInfo TrigSteeringEvent InDetByteStreamErrors  ITkPixelByteStreamCnvLib)


atlas_add_test(ITkPixQCoreEncodingLUT_test
  SOURCES test/ITkPixQCoreEncodingLUT_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} ITkPixelByteStreamCnvLib
  POST_EXEC_SCRIPT noerror.sh
)

atlas_add_test( ITkPixelRawContByteStreamCnvTest
  SCRIPT ITkPixelRawContByteStreamCnvTestJO.py
   POST_EXEC_SCRIPT noerror.sh )

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(ITkPixelByteStreamCnvLib INTERFACE -fconstexpr-ops-limit=200000000)
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(ITkPixelByteStreamCnvLib INTERFACE -fconstexpr-steps=200000000)
endif()
# Install files from the package:
atlas_install_runtime( share/*.csv )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test(PixelEncodingDecodingTest
  SCRIPT ITkPixelEncodingDecodingTest.py
  POST_EXEC_SCRIPT noerror.sh
  PROPERTIES SKIP_RETURN_CODE 2)

atlas_add_test(PixelEncodingDecodingPhaseIIRDOTest
  SCRIPT ITkPixelEncodingDecodingPhaseIIRDOTest.py
  POST_EXEC_SCRIPT noerror.sh
  PROPERTIES SKIP_RETURN_CODE 2)
