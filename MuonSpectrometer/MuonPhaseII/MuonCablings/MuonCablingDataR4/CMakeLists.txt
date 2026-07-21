atlas_subdir( MuonCablingData )

atlas_add_library(
    MuonCablingData
    src/*.cxx

    PUBLIC_HEADERS
    MuonCablingData

    LINK_LIBRARIES
    AthenaKernel
    GaudiKernel
    Identifier
    MuonIdHelpersLib
)
