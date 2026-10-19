# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkAlignGenTools )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Gpad Graf Matrix )

# Component(s) in the package:
atlas_add_library( TrkAlignGenToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS TrkAlignGenTools
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthContainers AthenaBaseComps BeamSpotConditionsData EventPrimitives GaudiKernel GeoPrimitives InDetAlignGenToolsLib InDetTrackSelectionToolLib StoreGateLib TrkAlignEvent TrkAlignInterfaces TrkEventPrimitives TrkExInterfaces TrkFitterInterfaces TrkFitterUtils TrkToolInterfaces TrkTrack TrkVertexFitterInterfaces xAODTracking
                   PRIVATE_LINK_LIBRARIES AtlasDetDescr TrkAlgebraUtils TrkCompetingRIOsOnTrack TrkDetElementBase TrkEventUtils TrkMaterialOnTrack TrkMeasurementBase TrkParameters TrkParticleBase TrkPseudoMeasurementOnTrack TrkRIO_OnTrack TrkSurfaces TrkTrackSummary TrkVertexOnTrack muonEvent xAODEventInfo )

atlas_add_component( TrkAlignGenTools
                     src/components/*.cxx
                     LINK_LIBRARIES TrkAlignGenToolsLib )
