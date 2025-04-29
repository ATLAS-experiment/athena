# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( PixelConditionsData )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( PixelConditionsData
                   src/*.cxx
                   PUBLIC_HEADERS PixelConditionsData
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES AthenaKernel AthenaPoolUtilities GeoPrimitives Identifier InDetByteStreamErrors InDetIdentifier PixelReadoutDefinitionsLib PixelReadoutGeometryLib CxxUtils PathResolver
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} )
                   
atlas_add_test( PixelChargeCondData_test
                SOURCES test/PixelChargeCondData_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES AthenaKernel PixelConditionsData ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh)
                
atlas_add_test( PixelModuleDataStream_test
                SOURCES test/PixelModuleDataStream_test.cxx 
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES PixelConditionsData  ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh)
                
atlas_add_test( PixelChargeCalibParameters_test
                SOURCES test/PixelChargeCalibParameters_test.cxx 
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES PixelConditionsData ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh)
                
atlas_add_test( PixelDCSHVData_test
                SOURCES test/PixelDCSHVData_test.cxx 
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES PixelConditionsData ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh)
                
atlas_install_runtime (test/*.txt)



