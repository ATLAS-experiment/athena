################################################################################
# Package: MuonPatternRecognitionAlgs
################################################################################

# Declare the package name:
atlas_subdir( MuonTruthSegmentMaker )


atlas_add_component( MuonTruthSegmentMaker
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  xAODMuonSimHit AthenaBaseComps GaudiKernel xAODMuon MuonIdHelpersLib
                                     MuonReadoutGeometryR4)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
