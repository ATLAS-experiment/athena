################################################################################
# Package: RpcDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( RpcDigitizationR4 )


atlas_add_component( RpcDigitizationR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4
                                     MuonDigitizationR4 MuonCondData MuonDigitContainer MuonIdHelpersLib TruthUtils)
