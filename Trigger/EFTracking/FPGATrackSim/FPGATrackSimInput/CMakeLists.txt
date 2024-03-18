# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

atlas_subdir( FPGATrackSimInput )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrackSimInputLib
   src/*.cxx
   PUBLIC_HEADERS            FPGATrackSimInput
   INCLUDE_DIRS              ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES            ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel InDetReadoutGeometry
                             FPGATrackSimConfToolsLib FPGATrackSimMapsLib FPGATrackSimObjectsLib
   PRIVATE_LINK_LIBRARIES    AthContainers InDetIdentifier  StoreGateLib FPGATrackSimSGInputLib )

atlas_add_component( FPGATrackSimInput
   src/components/*.cxx
   LINK_LIBRARIES            FPGATrackSimInputLib
)

# Install files from the package:
atlas_install_joboptions( share/*.py test/*.py)
atlas_install_scripts( scripts/*)
atlas_install_python_modules( share/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL900,ATL901 ) 

# Test in the package:
atlas_add_test(             testReadRawHit
  SCRIPT                    test/test_FPGATrackSimReadRawHitsWrapperAlg.sh
  LOG_SELECT_PATTERN        "FPGATrackSimRawHitsWrapp"
  PRIVATE_WORKING_DIRECTORY
)

# The output of this test deliberatety contains ERROR messages.
atlas_add_test(             testRawToLogical
  SCRIPT                    test/test_FPGATrackSimRawToLogicalWrapperAlg.sh 
  LOG_SELECT_PATTERN        "FPGATrackSimReadLogical"
  PRIVATE_WORKING_DIRECTORY
)
