# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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

atlas_add_test( L1CaloPhase1_Data
        SCRIPT athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data22_13p6TeV.00440499.physics_Main.daq.RAW._lb0461._SFO-13._0004.data --evtMax 10
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )

# Test the han config file generating mode of this application
# which is triggered by running an evtMax = 0 job.
atlas_add_test( L1CaloPhase1_ConfigGenerating
        SCRIPT "athena TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data22_13p6TeV.00440499.physics_Main.daq.RAW._lb0461._SFO-13._0004.data --evtMax 0"
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )