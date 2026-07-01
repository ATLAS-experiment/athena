# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelDigitization )

# External dependencies:
find_package( CLHEP )
find_package( Boost COMPONENTS unit_test_framework)
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread Matrix TreePlayer )

# Component(s) in the package:
atlas_add_component( PixelDigitization
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AtlasHepMCLib GaudiKernel GeneratorObjects HitManagement Identifier InDetCondTools InDetConditionsSummaryService InDetIdentifier InDetRawData InDetReadoutGeometry InDetSimData InDetSimEvent PathResolver PileUpToolsLib PixelConditionsData PixelReadoutDefinitionsLib PixelReadoutGeometryLib ReadoutGeometryBase SiDigitization SiPropertiesToolLib StoreGateLib TruthUtils )

atlas_add_test( BichselData_test
   SOURCES test/BichselData_test.cxx  src/BichselData.cxx src/PixelDigitizationUtilities.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} CxxUtils PixelConditionsData
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( PixelDigitizationUtilities_test
   SOURCES test/PixelDigitizationUtilities_test.cxx src/PixelDigitizationUtilities.cxx src/BichselData.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} CxxUtils PathResolver PixelConditionsData
   POST_EXEC_SCRIPT noerror.sh )
   
atlas_add_test( PixelNoiseFunctions_test
   SOURCES test/PixelNoiseFunctions_test.cxx src/PixelNoiseFunctions.cxx
   INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
   LINK_LIBRARIES ${Boost_LIBRARIES} ${CLHEP_LIBRARIES} CxxUtils InDetIdentifier InDetSimEvent PixelConditionsData PixelReadoutGeometryLib ReadoutGeometryBase SiDigitization
   POST_EXEC_SCRIPT noerror.sh )



# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_data( test/BichselTest.dat )
