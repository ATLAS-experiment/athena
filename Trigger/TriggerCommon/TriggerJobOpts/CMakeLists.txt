# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TriggerJobOpts )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-select=ATL900,ATL901 )
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( TriggerConfigFlagsTest
   SCRIPT python -m TriggerJobOpts.TriggerConfigFlags --verbose
   POST_EXEC_SCRIPT nopost.sh )

if (NOT XAOD_ANALYSIS)

   atlas_add_test( TriggerConfigTest
      SCRIPT python -m TriggerJobOpts.TriggerConfig
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( TriggerConfigFlags_AutoConfTest
      SCRIPT test_TriggerFlags_autoconf.py
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Lvl1SimulationConfig
      SCRIPT python -m TriggerJobOpts.Lvl1SimulationConfig
      PRIVATE_WORKING_DIRECTORY
      PROPERTIES TIMEOUT 600
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Lvl1MuonSimulationConfig
      SCRIPT python -m TriggerJobOpts.Lvl1MuonSimulationConfig
      PRIVATE_WORKING_DIRECTORY
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Run1TriggerRecoConfigTest
      SCRIPT python -m TriggerJobOpts.TriggerRecoConfig --config-only -- Trigger.EDMVersion=1
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Run2TriggerRecoConfigTest 
      SCRIPT python -m TriggerJobOpts.TriggerRecoConfig --config-only -- Trigger.EDMVersion=2
      POST_EXEC_SCRIPT noerror.sh )

   atlas_add_test( Run3TriggerRecoConfigTest
      SCRIPT python -m TriggerJobOpts.TriggerRecoConfig --config-only -- Trigger.EDMVersion=3
      POST_EXEC_SCRIPT noerror.sh )
endif()
