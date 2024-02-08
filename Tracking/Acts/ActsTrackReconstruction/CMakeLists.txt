# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrackReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( Boost )

atlas_add_component( ActsTrackReconstruction
                     src/*.h src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
		       ${Boost_INCLUDE_DIRS}
                     LINK_LIBRARIES
		       ${Boost_LIBRARIES}
                       ActsCore
		       ActsEventLib
		       ActsEventCnvLib
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
		       ActsInteropLib
		       ActsToolInterfacesLib
		       AthenaBaseComps
		       AthenaMonitoringKernelLib
		       BeamSpotConditionsData
		       GaudiKernel
		       GeoPrimitives
		       Identifier
		       InDetIdentifier
		       InDetPrepRawData
		       InDetRIO_OnTrack
		       InDetReadoutGeometry
		       MagFieldConditions
		       MagFieldElements
		       StoreGateLib
		       TRT_ReadoutGeometry
		       TrkEventPrimitives
		       TrkFitterInterfaces
		       TrkMeasurementBase
		       TrkParameters
		       TrkPrepRawData
		       TrkRIO_OnTrack
		       TrkSurfaces
		       TrkToolInterfaces
		       TrkTrack
		       TrkTrackSummary
		       TrkTruthData
		       TruthUtils
		       xAODEventInfo
		       xAODInDetMeasurement
		       xAODMeasurementBase
		       xAODTracking
                     )

# Install files from the package:
atlas_install_joboptions( share/*.py )
