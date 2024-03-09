# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelConditionsAlgorithms )

# External dependencies:
find_package( CLHEP )
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core RIO )
find_package( nlohmann_json )
find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_component( PixelConditionsAlgorithms
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${CORAL_LIBRARIES} ${ROOT_LIBRARIES} nlohmann_json::nlohmann_json AthenaBaseComps AthenaKernel AthenaPoolUtilities DetDescrConditions GaudiKernel GeoModelUtilities GeoPrimitives Identifier InDetCondTools InDetIdentifier InDetReadoutGeometry PathResolver PixelConditionsData PixelReadoutGeometryLib StoreGateLib TrkGeometry TrkSurfaces MuonReadoutGeometry TRT_ReadoutGeometry)

atlas_add_test( StringUtilities_test
  SOURCES test/StringUtilities_test.cxx src/StringUtilities.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES}
  POST_EXEC_SCRIPT "nopost.sh"
)

atlas_add_test( PixelModuleConfigCondAlg_test
  SOURCES test/PixelModuleConfigCondAlg_test.cxx src/PixelModuleConfigCondAlg.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps GaudiKernel AthenaPoolUtilities PixelConditionsData PathResolver StoreGateLib
  POST_EXEC_SCRIPT "nopost.sh"
)

atlas_add_test( PixelConfigCondAlg_test
  SOURCES test/PixelConfigCondAlg_test.cxx src/PixelConfigCondAlg.cxx src/StringUtilities.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps GaudiKernel AthenaPoolUtilities PixelConditionsData PathResolver StoreGateLib
  POST_EXEC_SCRIPT "nopost.sh"
)

atlas_add_test( ITkPixChargeCalibAlg_test
  SOURCES test/ITkPixChargeCalibAlg_test.cxx src/ITkPixChargeCalibAlg.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps GaudiKernel AthenaPoolUtilities PixelConditionsData PathResolver StoreGateLib
  POST_EXEC_SCRIPT "nopost.sh"
)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions(share/*.txt)
