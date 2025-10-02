# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist )

# Component(s) in the package:
atlas_add_component( TrigT1CaloMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel AthenaMonitoringKernelLib AthenaMonitoringLib CaloDetDescrLib CaloEvent CaloIdentifier FourMomUtils Identifier LArCablingLib LArRecConditions PathResolver SGTools StoreGateLib TrigConfData TrigDecisionToolLib TrigT1CaloCalibConditions TrigT1CaloEventLib TrigT1CaloMonitoringToolsLib TrigT1CaloToolInterfaces TrigT1CaloUtilsLib TrigT1Interfaces TrigT1Result xAODJet xAODTrigL1Calo xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py share/L1CaloPhase1Monitoring.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )
atlas_install_data( share/hanConfig_header.txt )
atlas_install_scripts( scripts/cmd-l1calo-dq-*.py scripts/l1calo-ath-mon )
atlas_add_alias( l1calo-dq-report "cmd-l1calo-dq-report.py" )
atlas_add_alias( l1calo-dq-web "cmd-l1calo-dq-web.py" )
atlas_add_alias( l1calo-dq-file "cmd-l1calo-dq-file.py" )
atlas_add_alias( l1calo-dq-test "cmd-l1calo-dq-test.py" )

atlas_add_test( L1CaloPhase1_Data
        SCRIPT athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt --evtMax 10
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )

# Test the han config file generating mode of this application
# which is triggered by running an evtMax = 0 job.
atlas_add_test( L1CaloPhase1_ConfigGenerating
        SCRIPT "athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt --evtMax 0"
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( L1CaloPhase1_Data_oneFex
        SCRIPT athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt --evtMax 10 -- Trigger.L1.doeFex=True Trigger.L1.doCaloInputs=True
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )
