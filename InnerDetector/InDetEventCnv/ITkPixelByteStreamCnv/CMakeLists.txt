# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkPixelByteStreamCnv )

# External dependencies:
find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_library( ITkPixelByteStreamCnvLib
   ITkPixelByteStreamCnv/*.h
   INTERFACE
   PUBLIC_HEADERS ITkPixelByteStreamCnv
   LINK_LIBRARIES GaudiKernel ByteStreamData InDetRawData )

atlas_add_component( ITkPixelByteStreamCnv
   src/*.cxx
   src/components/*.cxx
   LINK_LIBRARIES AthenaBaseComps ByteStreamData GaudiKernel ITkPixelByteStreamCnvLib InDetIdentifier InDetRawData InDetReadoutGeometry PixelReadoutGeometryLib StoreGateLib )

atlas_add_test(ITkPixQCoreEncodingLUT_test
  SOURCES test/ITkPixQCoreEncodingLUT_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} ITkPixelByteStreamCnvLib
  POST_EXEC_SCRIPT "nopost.sh"
)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  target_compile_options(ITkPixelByteStreamCnvLib INTERFACE -fconstexpr-ops-limit=200000000)
endif()
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options(ITkPixelByteStreamCnvLib INTERFACE -fconstexpr-steps=200000000)
endif()

# Install files from the package:
atlas_install_joboptions( share/*.txt )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
