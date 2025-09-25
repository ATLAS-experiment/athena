# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkPixelCabling )

# External dependencies:
find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( ITkPixelCablingLib
                   src/ITkPixelOnlineId.cxx src/ITkPixelCablingData.cxx
                   PUBLIC_HEADERS ITkPixelCabling
                   LINK_LIBRARIES AthenaKernel Identifier InDetReadoutGeometry PixelReadoutGeometryLib)

atlas_add_component( ITkPixelCabling
                     src/ITkPixelCablingAlg.cxx src/components/*.cxx 
                     LINK_LIBRARIES ITkPixelCablingLib InDetReadoutGeometry PixelReadoutGeometryLib
                     PRIVATE_LINK_LIBRARIES AthenaBaseComps GaudiKernel Identifier InDetIdentifier PathResolver StoreGateLib InDetReadoutGeometry PixelReadoutGeometryLib)

atlas_install_joboptions(share/*.txt)
atlas_install_runtime(share/*.dat)
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# Tests in the package:
atlas_add_test( ITkPixelOnlineId_test	
  SOURCES 
  test/ITkPixelOnlineId_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} ITkPixelCablingLib
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( ITkPixelCablingData_test	
  SOURCES 
  test/ITkPixelCablingData_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} ITkPixelCablingLib
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( ITkPixelCablingAlg_test	
  SOURCES 
  src/ITkPixelCablingAlg.cxx src/components/*.cxx test/ITkPixelCablingAlg_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps GaudiKernel
    PathResolver StoreGateLib Identifier IdDictParser ITkPixelCablingLib
    InDetIdentifier
  POST_EXEC_SCRIPT nopost.sh
)
