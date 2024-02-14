# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ISF_Algorithms )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( GTest )
find_package( GMock )
set( extra_libs )
if( NOT SIMULATIONBASE )
   set( extra_libs EventBookkeeperToolsLib )
endif()

# Component(s) in the package:
atlas_add_component( ISF_Algorithms
   src/*.h src/*.cxx src/components/*.cxx
   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES  ${ROOT_LIBRARIES} GaudiKernel AtlasHepMCLib CxxUtils
   AthenaBaseComps StoreGateLib AtlasDetDescr GeneratorObjects HepMC_InterfacesLib
   ISF_Event ISF_InterfacesLib ISF_HepMC_Interfaces PmbCxxUtils InDetSimEvent
   CaloSimEvent LArSimEvent TileSimEvent MuonSimEvent TrackRecordLib
   ${extra_libs} )

atlas_add_test( CollectionMerger_test
   SOURCES test/CollectionMerger_test.cxx src/CollectionMerger.h
   src/CollectionMerger.cxx
   INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} #${GMOCK_INCLUDE_DIRS}
   LINK_LIBRARIES ${GTEST_LIBRARIES} AthenaBaseComps CxxUtils InDetSimEvent LArSimEvent
   MuonSimEvent StoreGateLib TestTools TileSimEvent #${GMOCK_LIBRARIES}
   POST_EXEC_SCRIPT nopost.sh
   ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )

atlas_add_test(collection_merger_helpers_CA_tests
               SCRIPT python/test/test_CollectionMergerConfig.py
               POST_EXEC_SCRIPT nopost.sh)

atlas_add_test( SimKernelMT_test
                SOURCES
                test/SimKernelMT_test.cxx
                src/SimKernelMT.cxx
                INCLUDE_DIRS
                ${GTEST_INCLUDE_DIRS}
                ${GMOCK_INCLUDE_DIRS}
                LINK_LIBRARIES ${GMOCK_LIBRARIES} ${GTEST_LIBRARIES} AthenaBaseComps AtlasDetDescr AtlasHepMCLib
                CxxUtils GaudiKernel GeneratorObjects GeoPrimitives HepMC_InterfacesLib ISF_Event ISF_InterfacesLib
                StoreGateLib TrackRecordLib
                POST_EXEC_SCRIPT nopost.sh
                ENVIRONMENT
                "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/test"
                )
# Needed for the plugin service to see the test components
# defined in the test binary.
set_target_properties( ISF_Algorithms_SimKernelMT_test PROPERTIES ENABLE_EXPORTS True )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
