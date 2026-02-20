# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsVertexReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( ActsVertexReconstructionLib
                   src/*.cxx
                   PUBLIC_HEADERS ActsVertexReconstruction
                   LINK_LIBRARIES Acts::Core ActsGeometryLib ActsGeometryInterfacesLib ActsInteropLib AthenaBaseComps GaudiKernel BeamSpotConditionsData InDetRecToolInterfaces InDetTrackSelectionToolLib TrkLinks xAODInDetMeasurement xAODTracking)

atlas_add_component( ActsVertexReconstruction
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
		       Acts::Core
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
					 ActsVertexReconstructionLib
		       AthenaBaseComps
		       BeamSpotConditionsData
		       GaudiKernel
		       InDetRecToolInterfaces
		       InDetTrackSelectionToolLib
		       TrkLinks
		       TrkParticleBase
		       TrkTrack
		       TrkTrackLink
					 xAODInDetMeasurement
		       xAODTracking )
