################################################################################
# Package: TgcDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( TgcDigitizationR4 )


atlas_add_component( TgcDigitizationR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4
                                     MuonDigitizationR4 MuonCondData MuonDigitContainer MuonIdHelpersLib TruthUtils)
