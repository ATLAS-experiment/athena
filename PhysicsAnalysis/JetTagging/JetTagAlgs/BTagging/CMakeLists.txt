# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BTagging )

# Component(s) in the package:
atlas_add_library( BTaggingLib
                   src/BTagTool.cxx
                   src/BTagLightSecVertexing.cxx
                   src/JetBTaggingAlg.cxx
                   src/JetSecVertexingAlg.cxx
                   src/JetSecVtxFindingAlg.cxx
                   src/BTagTrackAugmenterAlg.cxx
                   PUBLIC_HEADERS BTagging
                   LINK_LIBRARIES AsgTools AthContainers AthenaBaseComps CxxUtils FlavorTagDiscriminants GaudiKernel GeoPrimitives JetInterface JetRecLib JetTagToolsLib MagFieldConditions ParticleJetToolsLib StoreGateLib TrkExInterfaces TrkVertexFitterInterfaces VxSecVertex VxVertex xAODBTagging xAODEventInfo xAODJet xAODMuon xAODTracking
                   PRIVATE_LINK_LIBRARIES InDetRecToolInterfaces JetTagEvent Particle TrkLinks TrkSurfaces VxJetVertex xAODBase xAODCore )

atlas_add_component( BTagging
                     src/components/*.cxx
                     LINK_LIBRARIES BTaggingLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# install test scripts
atlas_install_scripts(
  bin/testBTagging
  )
atlas_add_test(BTaggingTest
  SCRIPT testBTagging --n-events 20 -o slow.h5
  POST_EXEC_SCRIPT nopost.sh )
atlas_add_test(FasterBTaggingTest
  SCRIPT testBTagging --n-events 20 --fast -o fast.h5
  POST_EXEC_SCRIPT nopost.sh )
