# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AthViews )

# External package dependencies:
find_package( GTest )

# Component(s) in the package:
atlas_add_library( AthViews
                   src/*.cxx
                   PUBLIC_HEADERS AthViews
                   LINK_LIBRARIES AthContainers AthLinks AthenaKernel GaudiKernel SGTools StoreGateLib TrigSteeringEvent
                   PRIVATE_LINK_LIBRARIES AthenaBaseComps )

atlas_add_component( AthViewsAlgs
                     src/components/*.cxx
                     LINK_LIBRARIES AthViews )

atlas_add_component( AthViewsDFlow
                     src_dflow/*.cxx
                     src_dflow/components/*.cxx
                     LINK_LIBRARIES AthExHiveLib AthViews AthenaBaseComps AthenaKernel StoreGateLib )

atlas_add_dictionary( AthViewsAlgsDict
                      AthViews/AthViewsWriteDict.h
                      AthViews/selection.xml
                      LINK_LIBRARIES AthLinks TrigSteeringEvent
                      ELEMENT_LINKS TrigRoiDescriptorCollection )

# Tests in the package:
atlas_add_test( ViewLinking_test
                SOURCES test/ViewLinking_test.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} AthViews AthenaKernel GaudiKernel StoreGateLib TestTools )

atlas_add_test( ViewCollectionMerge_test
                SOURCES test/ViewCollectionMerge_test.cxx
                INCLUDE_DIRS ${GMOCK_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GMOCK_LIBRARIES} ${GTEST_LIBRARIES} AthContainers AthLinks AthViews AthenaBaseComps AthenaKernel CxxUtils GaudiKernel GoogleTestTools StoreGateLib TrigSteeringEvent
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/test"
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( SimpleViews
                SCRIPT test/SimpleViewExample.py --threads=1
                PROPERTIES PASS_REGULAR_EXPRESSION "INFO view_test running with store view_9"
                POST_EXEC_SCRIPT nopost.sh )

atlas_add_test( ViewScheduling
                SCRIPT test/ViewScheduling.py --threads=1
                LOG_SELECT_PATTERN "INFO condTestAlg running with store view_0|INFO 109" )

atlas_add_test( ViewInheritance
                SCRIPT test/ViewInheritance.py --threads=1
                POST_EXEC_SCRIPT nopost.sh )
