# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SCT_Monitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist )

# Component(s) in the package:
atlas_add_library( SCT_MonitoringLib
                   SCT_Monitoring/*.h
                   INTERFACE
                   PUBLIC_HEADERS SCT_Monitoring
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} )

atlas_add_component( SCT_Monitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AthContainers AthenaKernel AthenaMonitoringKernelLib AthenaMonitoringLib CommissionEvent GaudiKernel Identifier InDetConditionsSummaryService InDetIdentifier InDetPrepRawData InDetRIO_OnTrack InDetRawData InDetReadoutGeometry LumiBlockData MagFieldConditions MagFieldElements SCT_ConditionsToolsLib SCT_MonitoringLib SCT_ReadoutGeometry StoreGateLib TrkEventUtils TrkMeasurementBase TrkParameters TrkRIO_OnTrack TrkSpacePoint TrkSurfaces TrkToolInterfaces TrkTrack TrkTrackSummary xAODEventInfo )

atlas_add_dictionary( SCT_MonitoringDict
                      SCT_Monitoring/SCT_MonitoringDict.h
                      SCT_Monitoring/selection.xml )

# Run tests:
# This is a standalone test with ESD input.
# This is not essential and can be abandoned if it won't run.
atlas_add_test( SCTLorentzMonAlg_test
                SCRIPT python -m SCT_Monitoring.SCTLorentzMonAlg
                POST_EXEC_SCRIPT noerror.sh
                PROPERTIES TIMEOUT 1000 )

# Install files from the package:
atlas_install_python_modules( python/*.py )
