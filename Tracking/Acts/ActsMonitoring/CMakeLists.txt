# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsMonitoring )

# External dependencies
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ActsMonitoring
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
                Acts::Core
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
		ActsTruthLib
                InDetMeasurementUtilitiesLib
                HGTD_ReadoutGeometry
                MuonTesterTreeLib
                TrkSurfaces )

atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_add_test( ActsSeedingToolComparison
  SCRIPT test/ActsSeedingAlgorithmTest.py
  PROPERTIES TIMEOUT 900
  POST_EXEC_SCRIPT nopost.sh )

file(GLOB_RECURSE files "util/*.cxx")
foreach(_exeFile ${files})
  get_filename_component(_theExec ${_exeFile} NAME_WE)
  get_filename_component(_theLoc ${_exeFile} DIRECTORY)
  # we specify a folder for programs we do not want to compile. Useful during r21 transition...
  if(${_theLoc} MATCHES "DoNotBuild")
    continue()
  endif() 
  atlas_add_executable( ${_theExec} ${_exeFile}
                       INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                       LINK_LIBRARIES ${ROOT_LIBRARIES}  PathResolver Acts::Core  ActsGeometryInterfacesLib GeoPrimitives)
 
endforeach()