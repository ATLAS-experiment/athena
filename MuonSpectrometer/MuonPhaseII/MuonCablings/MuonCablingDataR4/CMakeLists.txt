atlas_subdir( MuonCablingDataR4 )

atlas_add_library(
	MuonCablingDataR4
    src/*.cxx

    PUBLIC_HEADERS
    MuonCablingDataR4

    LINK_LIBRARIES
    AthenaKernel
    GaudiKernel
    Identifier
    MuonIdHelpersLib
)
