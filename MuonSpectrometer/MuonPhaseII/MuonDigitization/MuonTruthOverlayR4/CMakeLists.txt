################################################################################
# Package: MuonTruthOverlayR4
################################################################################

# Declare the package name:
atlas_subdir( MuonTruthOverlayR4 )


atlas_add_component( MuonTruthOverlayR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4 MuonIdHelpersLib xAODMuonViews TruthUtils)

atlas_install_python_modules( python/*.py  POST_BUILD_CMD ${ATLAS_FLAKE8} )
