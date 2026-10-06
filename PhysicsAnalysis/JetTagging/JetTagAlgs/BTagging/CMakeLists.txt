# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( BTagging )

# Component(s) in the package:
atlas_add_library( BTaggingLib
                   src/JetSecVertexingAlg.cxx
                   src/JetSecVtxFindingAlg.cxx
                   src/BTagTrackAugmenterAlg.cxx
                   src/JetTagVertexDecoratorAlg.cxx
                   src/BTagTrackAugmenterByVertexAlg.cxx
                   PUBLIC_HEADERS BTagging
                   LINK_LIBRARIES AthContainers AthLinks AthenaBaseComps CxxUtils GaudiKernel GeoPrimitives InDetRecToolInterfaces JetTagToolsLib MagFieldConditions StoreGateLib TrkExInterfaces TrkVertexFitterInterfaces VxSecVertex VxVertex xAODBTagging xAODJet xAODMuon xAODTracking
                   PRIVATE_LINK_LIBRARIES TrkLinks TrkSurfaces VxJetVertex xAODBase xAODCore )

atlas_add_component( BTagging
                     src/components/*.cxx
                     LINK_LIBRARIES BTaggingLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

atlas_install_scripts(
  bin/testFlavorTagging
)

# install test scripts
atlas_add_test(FlavorTaggingTest
  SCRIPT testFlavorTagging --n-events 20 -o slow.h5
  # JetH5Writer explicitly creates NANs.
  LOG_IGNORE_PATTERN "FPE INVALID"
  POST_EXEC_SCRIPT noerror.sh )
atlas_add_test(FasterBTaggingTest
  SCRIPT testFlavorTagging --n-events 20 --fast -o fast.h5
  # JetH5Writer explicitly creates NANs.
  LOG_IGNORE_PATTERN "FPE INVALID"
  POST_EXEC_SCRIPT noerror.sh )
