# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TRTMonitoringRun3 )

# Component(s) in the package:
atlas_add_component( TRTMonitoringRun3
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthContainers AthenaMonitoringKernelLib AthenaMonitoringLib AtlasDetDescr CommissionEvent EventPrimitives GaudiKernel InDetByteStreamErrors InDetConditionsSummaryService InDetIdentifier InDetRIO_OnTrack InDetRawData InDetTrackSelectionToolLib StoreGateLib TRT_ConditionsServicesLib TRT_DriftFunctionToolLib TRT_ReadoutGeometry TrkToolInterfaces TrkTrack TrkTrackSummary xAODEventInfo xAODTrigger CxxUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py )
