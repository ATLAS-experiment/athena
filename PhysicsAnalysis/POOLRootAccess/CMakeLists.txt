# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( POOLRootAccess )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( POOLRootAccessLib
                   src/*.cxx
                   PUBLIC_HEADERS POOLRootAccess
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaKernel GaudiKernel SGTools StoreGateLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthAnalysisBaseCompsLib StoreGateBindings )

atlas_add_dictionary( POOLRootAccessDict
                      POOLRootAccess/POOLRootAccessDict.h
                      POOLRootAccess/selection.xml
                      LINK_LIBRARIES POOLRootAccessLib )


		    
atlas_add_test( ut_basicRead_test
                SOURCES test/ut_basicRead_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AthAnalysisBaseCompsLib POOLRootAccessLib xAODEventInfo xAODRootAccess CxxUtils
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 30 )

atlas_add_test( ut_basicxAODRead_test
                 SOURCES test/ut_basicxAODRead_test.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} POOLRootAccessLib xAODBase xAODEventInfo xAODRootAccess CxxUtils
                      POST_EXEC_SCRIPT noerror.sh
                 PROPERTIES TIMEOUT 30 )

atlas_add_test( ut_basicAlg_test
                SOURCES test/ut_basicAlg_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} POOLRootAccessLib AthAnalysisBaseCompsLib xAODEventInfo xAODRootAccess
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 30 )

atlas_add_test( basicRead_py
                SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/ut_basicRead_test.py
                POST_EXEC_SCRIPT noerror.sh )

# Install files from the package:
atlas_install_joboptions( share/*.opts )

