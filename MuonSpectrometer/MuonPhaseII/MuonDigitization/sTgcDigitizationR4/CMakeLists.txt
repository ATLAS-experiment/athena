################################################################################
# Package: sTgcDigitizationR4
################################################################################

# Declare the package
atlas_subdir( sTgcDigitizationR4 )

# Add the main library with source files and public headers
atlas_add_library( sTgcDigitizationR4Lib
  src/*.cxx
  PUBLIC_HEADERS sTgcDigitizationR4
  LINK_LIBRARIES
    AthenaKernel
    StoreGateLib
    xAODMuonSimHit
    MuonReadoutGeometryR4
    MuonDigitizationR4
    MuonCondData
    MuonDigitContainer
    MuonIdHelpersLib
    NSWCalibToolsLib
    TruthUtils
)

# Add the component library for plugin entries (e.g. ToolFactory)
atlas_add_component( sTgcDigitizationR4
  src/components/*.cxx
  LINK_LIBRARIES sTgcDigitizationR4Lib
)
