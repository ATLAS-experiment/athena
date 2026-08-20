# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the project's name and version.
atlas_subdir( JetTagDerivationUtils )

# External dependencies:
find_package( Boost COMPONENTS json)
find_package( FastJet )

atlas_add_library(JetTagDerivationUtilsLib
  PUBLIC_HEADERS JetTagDerivationUtils
  )

atlas_add_component(JetTagDerivationUtils
  src/JetMatcherAlg.cxx
  src/FlowSelectorAlg.cxx
  src/MetadataAlg.cxx
  src/TrigBTagCopierAlg.cxx
  src/JetLinkMatcherAlg.cxx
  src/SubjetBuilderAlg.cxx
  src/TruthTauDecoratorAlg.cxx
  src/components/*.cxx
  INCLUDE_DIRS ${FASTJET_INCLUDE_DIRS}
  LINK_LIBRARIES
  ${FASTJET_LIBRARIES}
  AthContainers
  AthLinks
  AthenaBaseComps
  FourMomUtils
  StoreGateLib
  xAODBase
  xAODPFlow
  xAODJet
  xAODBTagging
  PMGAnalysisInterfacesLib
  xAODCutFlow
  IH5GroupSvc
  CutBookkeeperUtils
  HDF5Utils
  xAODMetaData
  xAODTruth
  ${Boost_LIBRARIES}
  )


atlas_install_python_modules(
  python/MetadataAlgConfig.py
  python/JetMatchingConfig.py
  python/TrigBTagCopierConfig.py
  python/CopyJetParentInfoConfig.py
  python/SubjetBuilderConfig.py
  python/TruthTauDecoratorConfig.py
  POST_BUILD_CMD ${ATLAS_FLAKE8}
  )
atlas_install_scripts(
  bin/ftag-test-metadata
  bin/ftag-test-metadata-merge
  bin/ftag-test-TrigCopy
  bin/ftag-test-subjets )

atlas_add_test( JetTagMetadataAlg
  SCRIPT ftag-test-metadata-merge
  POST_EXEC_SCRIPT noerror.sh )

# No atlas_add_test: ftag-test-subjets needs a jet collection with its ghost
# associations intact plus TruthTaus, which the standard test inputs lack.
