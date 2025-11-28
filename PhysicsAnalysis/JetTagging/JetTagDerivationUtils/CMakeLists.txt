# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Set the project's name and version.
atlas_subdir( JetTagDerivationUtils )

# External dependencies:
find_package( Boost COMPONENTS json)

atlas_add_library(JetTagDerivationUtilsLib
  PUBLIC_HEADERS JetTagDerivationUtils
  )

atlas_add_component(JetTagDerivationUtils
  src/JetMatcherAlg.cxx
  src/FlowSelectorAlg.cxx
  src/MetadataAlg.cxx
  src/TrigBTagCopierAlg.cxx
  src/components/*.cxx
  LINK_LIBRARIES
  AthenaBaseComps
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
  POST_BUILD_CMD ${ATLAS_FLAKE8}
  )
atlas_install_scripts( bin/ftag-test-metadata bin/ftag-test-TrigCopy )

atlas_add_test( JetTagMetadataAlg
  SCRIPT ftag-test-metadata -m 10 --h5-output test.h5 --json-output test.json
  POST_EXEC_SCRIPT nopost.sh )
