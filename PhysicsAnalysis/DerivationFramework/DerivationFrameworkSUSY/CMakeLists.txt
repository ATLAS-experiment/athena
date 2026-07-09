# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DerivationFrameworkSUSY )
find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_component( DerivationFrameworkSUSY
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AthenaKernel DerivationFrameworkInterfaces ExpressionEvaluationLib GaudiKernel  RecoToolInterfaces xAODAssociations xAODEventInfo xAODJet xAODTracking xAODTruth ReweightUtilsLib TruthUtils)

atlas_add_test (PdgConditional_test
                SOURCES src/PdgConditional.cxx test/PdgConditional_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh )
                
atlas_add_test (utilityFunctions_test
                SOURCES src/PdgConditional.cxx src/utilityFunctions.cxx test/utilityFunctions_test.cxx
                INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                LINK_LIBRARIES ${Boost_LIBRARIES}
                POST_EXEC_SCRIPT nopost.sh )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

