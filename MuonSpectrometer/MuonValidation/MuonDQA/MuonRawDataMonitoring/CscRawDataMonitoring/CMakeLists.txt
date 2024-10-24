# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CscRawDataMonitoring )

# Component(s) in the package:
atlas_add_component( CscRawDataMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaMonitoringLib AthenaMonitoringKernelLib GaudiKernel MuonPrepRawData MuonSegment TrkSegment TrkTrack TrigDecisionToolLib StoreGateLib xAODEventInfo MuonIdHelpersLib MuonRDO CscCalibToolsLib CscClusterizationLib MuonRIO_OnTrack MuonRecHelperToolsLib TrkSurfaces TrkCompetingRIOsOnTrack TrkEventPrimitives MuonCSC_CnvToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

