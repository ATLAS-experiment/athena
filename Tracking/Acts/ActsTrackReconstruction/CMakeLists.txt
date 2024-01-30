# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsTrackReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( ROOT COMPONENTS Core Tree RIO )

atlas_add_component( ActsTrackReconstruction
                     src/*.cxx
                     src/components/*.cxx
		     INCLUDE_DIRS
		       ${ROOT_INCLUDE_DIRS}	
                     LINK_LIBRARIES
		       ${ROOT_LIBRARIES}
                       GaudiKernel
                       BeamSpotConditionsData
                       ActsCore
                       ActsEventLib
                       ActsToolInterfacesLib
                       MagFieldConditions
                       ActsGeometryInterfacesLib
                       SiSPSeededTrackFinderData
		       AthenaMonitoringKernelLib
                       ActsEventCnvLib
		       ActsInteropLib
		       InDetRecToolInterfaces
		       xAODInDetMeasurement
		       SiSpacePoint		     
		       CxxUtils
		       ActsGeometryLib
		       EventPrimitives
		       GeoPrimitives
		       AthenaBaseComps
		       AthenaKernel
		       TrkTruthData
		       CaloDetDescrLib
		       EventInfo
		       StoreGateLib
		       TRT_ReadoutGeometry
                       TrkGeometry
                       TrkExInterfaces
                       TrkParameters
                       TrkSurfaces
                       TrkFitterUtils
                       TrkTrackSummary
		       xAODTracking
		       TrkFitterInterfaces
		       InDetRIO_OnTrack
		       TruthUtils
                     )

# Install files from the package:
atlas_install_joboptions( share/*.py )

