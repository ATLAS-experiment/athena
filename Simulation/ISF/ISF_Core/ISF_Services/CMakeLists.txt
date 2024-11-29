# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_Services )

# External dependencies:
find_package( CLHEP )
find_package( HepPDT )
find_package( GTest )
find_package( GMock )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( ISF_Services
                   src/*.cxx
                   src/components/*.cxx
                   OBJECT
                   NO_PUBLIC_HEADERS
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS} ${HEPPDT_INCLUDE_DIRS}
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AtlasHepMCLib ${CLHEP_LIBRARIES} ${HEPPDT_LIBRARIES} GaudiKernel BarcodeInterfacesLib AthenaBaseComps StoreGateLib SubDetectorEnvelopesLib AtlasDetDescr CxxUtils GeneratorObjects MCTruth ISF_Event ISF_HepMC_Interfaces ISF_InterfacesLib PmbCxxUtils TruthUtils )

# Tests in the package:
atlas_add_test( InputConverter_test
                SOURCES
                test/InputConverter_test.cxx
                src/InputConverter.cxx
                INCLUDE_DIRS
                ${CLHEP_INCLUDE_DIRS}
                ${HEPPDT_INCLUDE_DIRS}
                ${GTEST_INCLUDE_DIRS}
                ${GMOCK_INCLUDE_DIRS}
                LINK_LIBRARIES
                ${CLHEP_LIBRARIES}
                ${GMOCK_LIBRARIES}
                ${GTEST_LIBRARIES}
                ${HEPPDT_LIBRARIES}
                AthenaBaseComps
                AtlasHepMCLib
                CxxUtils
                GaudiKernel
                GeneratorObjects
                GeoPrimitives
                GoogleTestTools
                ISF_Event
                ISF_HepMC_Interfaces
                ISF_InterfacesLib
                MCTruth
                TruthUtils
                ENVIRONMENT
                "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/test"
                POST_EXEC_SCRIPT nopost.sh
                )
# Needed for the plugin service to see the test components
# defined in the test binary.
set_target_properties( ISF_Services_InputConverter_test PROPERTIES ENABLE_EXPORTS True )

atlas_add_test( TruthSvc_test
                SOURCES
                  test/TruthSvc_test.cxx src/TruthSvc.cxx
                INCLUDE_DIRS
                  ${GTEST_INCLUDE_DIRS}
                  ${GMOCK_INCLUDE_DIRS}
                  ${CLHEP_INCLUDE_DIRS}
                LINK_LIBRARIES
                  ${GTEST_LIBRARIES}
                  ${GMOCK_LIBRARIES}
                  ${CLHEP_LIBRARIES}
                  AtlasDetDescr
                  AtlasHepMCLib
                  CxxUtils
                  GaudiKernel
                  BarcodeInterfacesLib
                  AthenaBaseComps
                  GoogleTestTools
                  GeneratorObjects
                  StoreGateLib
                  ISF_Event
                  ISF_HepMC_Interfaces
                  ISF_InterfacesLib
                  TruthUtils
                ENVIRONMENT
                  "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/test"
                POST_EXEC_SCRIPT nopost.sh
                )

#test ISF_ServicesConfig
atlas_add_test( ISF_ServicesConfig_test
                SCRIPT test/ISF_ServicesConfig_test.py
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 300 )

# Needed for the plugin service to see the test components
# defined in the test binary.
set_target_properties( ISF_Services_TruthSvc_test PROPERTIES ENABLE_EXPORTS True )

# Turn on/off LTO for all targets in the package.
set_target_properties(
  ISF_Services
  ISF_Services_InputConverter_test
  ISF_Services_TruthSvc_test
  PROPERTIES
  INTERPROCEDURAL_OPTIMIZATION ${ATLAS_GEANT4_USE_LTO} )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
