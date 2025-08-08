# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsVertexReconstruction )

# External dependencies:
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( ActsVertexReconstruction
                     src/*.h src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES 
		       Acts::Core
		       ActsGeometryLib
		       ActsGeometryInterfacesLib
		       AthenaBaseComps
		       BeamSpotConditionsData
		       GaudiKernel
		       InDetRecToolInterfaces
		       InDetTrackSelectionToolLib
		       TrkLinks
		       TrkParticleBase
		       TrkTrack
		       TrkTrackLink
		       xAODTracking )
