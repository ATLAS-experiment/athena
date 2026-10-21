# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FlavourTaggingTests )

find_package( ROOT COMPONENTS Core Tree )

if (NOT XAOD_ANALYSIS)
  # Install files from the package:
  atlas_install_scripts( test/*.sh )
  atlas_install_scripts( share/test_*.sh )
endif()

# add the test utility
atlas_add_executable(
  test-btagging
  util/test-btagging.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} xAODJet xAODTracking xAODBTagging xAODRootAccess
  )

atlas_add_executable(
  test-jet-links
  util/test-jet-links.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} xAODJet xAODTracking xAODBTagging xAODRootAccess
  )

atlas_add_executable(
  test-associated-iparticles
  util/test-associated-iparticles.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} xAODJet xAODTracking xAODRootAccess
  )

atlas_add_executable(
  test-jet-constituents
  util/test-jet-constituents.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} xAODJet xAODPFlow xAODBase xAODRootAccess AthContainers
  )
