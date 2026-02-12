# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ExpressionEvaluation )

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( ExpressionEvaluationLib
                   src/*.cxx
                   Root/*.cxx
                   PUBLIC_HEADERS ExpressionEvaluation
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers CxxUtils GaudiKernel SGTools StoreGateLib TrigDecisionToolLib AthenaBaseComps RootUtils
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} AthAnalysisBaseCompsLib AthenaKernel )

# Test(s) in the package
atlas_add_test( ExpressionParser_test
                SOURCES test/ExpressionParser_test.cxx
                LINK_LIBRARIES AthenaKernel ExpressionEvaluationLib GaudiKernel StoreGateLib TestTools xAODMuon xAODTracking
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 300
                ENVIRONMENT "JOBOPTSEARCHPATH=${CMAKE_CURRENT_SOURCE_DIR}/share" )

atlas_add_test( PointerCache_test
                SOURCES test/PointerCache_test.cxx
                LINK_LIBRARIES ExpressionEvaluationLib TestTools
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 300 )
