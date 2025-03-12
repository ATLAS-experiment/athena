################################################################################
# Package: MmDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( MmDigitizationR4 )


atlas_add_component( MmDigitizationR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4
                                     MuonDigitizationR4 MuonCondData MuonDigitContainer MuonIdHelpersLib MagFieldConditions MagFieldElements NSWCalibToolsLib MM_DigitizationLib TruthUtils)
