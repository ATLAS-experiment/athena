# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetV0Finder )

atlas_add_library( InDetV0FinderLib
                   src/*.cxx
                   PUBLIC_HEADERS InDetV0Finder
                   LINK_LIBRARIES AthContainers AthenaBaseComps GaudiKernel StoreGateLib xAODTracking TrkVertexAnalysisUtilsLib BeamSpotConditionsData TruthUtils InDetConversionFinderToolsLib ITrackToVertex TrkExInterfaces TrkToolInterfaces TrkVertexFitterInterfaces MVAUtils PathResolver
                   PRIVATE_LINK_LIBRARIES GeneratorModulesLib TrkV0FitterLib TrkVKalVrtFitterLib xAODEventInfo )

atlas_add_component( InDetV0Finder
                     src/components/*.cxx
                     LINK_LIBRARIES InDetV0FinderLib )
