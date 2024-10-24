# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MdtRawDataMonitoring )

# External dependencies:
find_package( ROOT COMPONENTS Core Graf Hist )

# Component(s) in the package:
atlas_add_component( MdtRawDataMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} AnalysisTriggerEvent AsgTools AthenaBaseComps AthenaMonitoringKernelLib AthenaMonitoringLib GaudiKernel LWHists MdtCalibFitters MuonAnalysisInterfacesLib MuonCalibIdentifier MuonDQAUtilsLib MuonIdHelpersLib MuonPrepRawData MuonRIO_OnTrack MuonReadoutGeometry MuonSegment StoreGateLib TrkEventPrimitives TrkSegment TrkTrack xAODEventInfo xAODMuon xAODTracking xAODTrigger )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

