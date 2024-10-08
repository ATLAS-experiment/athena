# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( VrtSecInclusive )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )

# Component(s) in the package:
atlas_add_library( VrtSecInclusiveLib
                   src/*.cxx
                   PUBLIC_HEADERS VrtSecInclusive
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps xAODEventInfo xAODTracking xAODTruth xAODMuon xAODEgamma GaudiKernel GeneratorObjects ITrackToVertex TrkDetDescrInterfaces TrkSurfaces TrkExInterfaces InDetConditionsSummaryService TrkToolInterfaces TrkVertexFitterInterfaces TrkVKalVrtFitterLib InDetIdentifier
                   PRIVATE_LINK_LIBRARIES AthContainers EventPrimitives StoreGateLib TrkTrackSummary TrkVKalVrtCore VxVertex TruthUtils )

atlas_add_component( VrtSecInclusive
                     src/components/*.cxx
                     LINK_LIBRARIES VrtSecInclusiveLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
