################################################################################
# Package: MuonPatternRecognitionAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonPatternRecognitionAlgs )

# External ACTS dependency
find_package( Acts COMPONENTS Core )

atlas_add_component( MuonPatternRecognitionAlgs
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  MuonPatternEvent ActsCore AthenaKernel StoreGateLib MuonTesterTreeLib
                                    xAODMuonPrepData MuonSpacePoint )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
