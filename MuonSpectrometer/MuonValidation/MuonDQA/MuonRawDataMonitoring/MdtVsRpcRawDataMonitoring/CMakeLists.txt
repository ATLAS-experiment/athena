# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MdtVsRpcRawDataMonitoring )

# Component(s) in the package:
atlas_add_component( MdtVsRpcRawDataMonitoring
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaMonitoringLib StoreGateLib GaudiKernel MuonReadoutGeometry MuonIdHelpersLib MuonDQAUtilsLib MuonRDO MuonPrepRawData )

