# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkValTools )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Graf )

atlas_add_library( TrkValToolsLib
                   TrkValTools/*.h
                   INTERFACE
                   PUBLIC_HEADERS TrkValTools
                   LINK_LIBRARIES GaudiKernel AthenaBaseComps TrkValInterfaces TrkFitterInterfaces StoreGateLib xAODEventInfo TrkParameters TrkValEvent AtlasHepMCLib TrkEventPrimitives TrkTrack AthContainers TruthUtils)

# Component(s) in the package:
atlas_add_component( TrkValTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AtlasDetDescr AtlasHepMCLib CommissionEvent EventInfo EventPrimitives GaudiKernel GeneratorObjects TrkCompetingRIOsOnTrack TrkEventPrimitives TrkEventUtils TrkExInterfaces TrkFitterUtils TrkMaterialOnTrack TrkMeasurementBase TrkParameters TrkParticleBase TrkRIO_OnTrack TrkSurfaces TrkToolInterfaces TrkTrack TrkTrackSummary TrkTruthData TrkValEvent TrkValInterfaces TrkValToolsLib )
