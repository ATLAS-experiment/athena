################################################################################
# Package: MuonPatternRecognitionAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionAlgs )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore Minuit2 Gpad Graf Hist HistPainter )


atlas_add_component( MuonPatternRecognitionAlgs
                     src/components/*.cxx src/*.cxx
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} MuonPatternEvent AthenaKernel StoreGateLib MuonTesterTreeLib
                                    xAODMuonPrepData MuonSpacePoint MuonPatternHelpers MuonSpacePointCalibratorLib MuonTruthHelpers )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( test/*.sh )
