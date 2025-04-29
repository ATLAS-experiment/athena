################################################################################
# Package: sTgcDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( sTgcDigitizationR4 )


atlas_add_component( sTgcDigitizationR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4
                                     MuonDigitizationR4 MuonCondData MuonDigitContainer MuonIdHelpersLib TruthUtils)
