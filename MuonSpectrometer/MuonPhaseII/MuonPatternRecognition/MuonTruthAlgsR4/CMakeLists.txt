################################################################################
# Package: MuonTruthAlgsR4
################################################################################

# Declare the package name:
atlas_subdir( MuonTruthAlgsR4 )


atlas_add_component( MuonTruthAlgsR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  xAODMuonSimHit AthenaBaseComps GaudiKernel xAODMuon MuonIdHelpersLib
                                     MuonReadoutGeometryR4 xAODMuonPrepData MuonPatternEvent xAODMuonViews
                                     TrkTruthData MuonTruthHelpers MuonRecHelperToolsLib TruthUtils DerivationFrameworkMuonsLib)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
