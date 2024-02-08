# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsPatternRecognition )

# External dependencies:
find_package( Acts COMPONENTS Core )

atlas_add_component( ActsPatternRecognition
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES
		       ActsCore
		       ActsEventLib
		       ActsEventCnvLib
		       ActsGeometryInterfacesLib
		       ActsInteropLib
		       ActsToolInterfacesLib
		       AthenaBaseComps
		       AthenaMonitoringKernelLib
		       BeamSpotConditionsData
		       CxxUtils
		       GaudiKernel
		       InDetPrepRawData
		       InDetReadoutGeometry
		       InDetRecToolInterfaces
		       MagFieldConditions
		       MagFieldElements
		       SiSPSeededTrackFinderData
		       SiSpacePoint
		       StoreGateLib
		       TrkEventUtils
		       TrkParameters
		       TrkSpacePoint
		       xAODInDetMeasurement )
