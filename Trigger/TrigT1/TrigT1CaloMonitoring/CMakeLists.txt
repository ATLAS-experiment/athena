# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1CaloMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Hist )

# Component(s) in the package:
atlas_add_component( TrigT1CaloMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaMonitoringKernelLib AthenaMonitoringLib AthenaPoolUtilities CaloDetDescrLib CaloIdentifier EventInfo GaudiKernel FourMomUtils Identifier LWHists SGTools StoreGateLib TrigAnalysisInterfaces TrigConfData TrigConfInterfaces TrigConfL1Data TrigDecisionToolLib TrigT1CaloCalibConditions TrigT1CaloCalibToolsLib TrigT1CaloCondSvcLib TrigT1CaloEventLib TrigT1CaloMonitoringToolsLib TrigT1CaloToolInterfaces TrigT1CaloUtilsLib TrigT1Interfaces xAODEgamma xAODTrigL1Calo xAODTrigger PathResolver )

# Install files from the package:
atlas_install_python_modules( python/*.py share/L1CaloPhase1Monitoring.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_joboptions( share/*.py )

atlas_add_test( L1CaloPhase1_Data
        SCRIPT athena --CA TrigT1CaloMonitoring/L1CaloPhase1Monitoring.py --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data22_13p6TeV.00440499.physics_Main.daq.RAW._lb0461._SFO-13._0004.data --evtMax 10
        PROPERTIES TIMEOUT 300
        PRIVATE_WORKING_DIRECTORY
        POST_EXEC_SCRIPT noerror.sh )