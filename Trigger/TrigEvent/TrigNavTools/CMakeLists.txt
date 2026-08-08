# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigNavTools )

# External dependencies:
find_package( Boost )

# Component(s) in the package:
atlas_add_library( TrigNavToolsLib
                   PUBLIC_HEADERS TrigNavTools
                   INTERFACE )

atlas_add_component( TrigNavTools
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES ${Boost_LIBRARIES} AthLinks AthenaBaseComps AthenaKernel CxxUtils GaudiKernel StoreGateLib TrigCompositeUtilsLib TrigConfHLTData TrigConfHLTUtilsLib TrigConfInterfaces TrigDecisionToolLib TrigNavStructure TrigNavigationLib TrigNavToolsLib TrigSteeringEvent TriggerMatchingToolLib xAODBase xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( NavConversionConfigTest
                SCRIPT python -m TrigNavTools.NavConverterConfig
                POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( ExecuteConversion
                SCRIPT testTrigR2ToR3NavGraphConversionV2.py --evtMax=200
                POST_EXEC_SCRIPT noerror.sh )
