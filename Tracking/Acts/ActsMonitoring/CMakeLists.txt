# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsMonitoring )

# External dependencies
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ActsMonitoring
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
                ActsCore
                ActsEventLib
                ActsGeometryLib
                ActsGeometryInterfacesLib
                ActsToolInterfacesLib
                AthenaMonitoringLib
                AthenaMonitoringKernelLib
                BeamSpotConditionsData
                GaudiKernel
                GeoPrimitives
                InDetIdentifier
                InDetPrepRawData
                InDetReadoutGeometry
                InDetRecToolInterfaces
                MagFieldConditions
                MagFieldElements
                PixelReadoutGeometryLib
                ReadoutGeometryBase
                SiSPSeededTrackFinderData
                StoreGateLib
                TrkTruthData
                TrkValHistUtils
                xAODEventInfo
                xAODInDetMeasurement
                xAODMeasurementBase
                InDetMeasurementUtilitiesLib
                HGTD_ReadoutGeometry )

atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( ActsSeedingToolComparison
  SCRIPT test/ActsSeedingAlgorithmTest.py
  PROPERTIES TIMEOUT 900
  POST_EXEC_SCRIPT nopost.sh )
