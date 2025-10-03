################################################################################
# Package: MdtDigitizationR4
################################################################################

# Declare the package name:
atlas_subdir( MdtDigitizationR4 )


atlas_add_component( MdtDigitizationR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES  AthenaKernel StoreGateLib xAODMuonSimHit MuonReadoutGeometryR4 MdtCalibInterfacesLib
                                     MuonDigitizationR4 MuonCondData MuonCablingData MuonDigitContainer MuonIdHelpersLib  
                                     MDT_DigitizationLib TruthUtils)
