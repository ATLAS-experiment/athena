atlas_subdir( MuonTGC_Cabling )

atlas_add_component(
    MuonTGC_Cabling
    src/TgcCablingAlg.cxx
    src/components/MuonTGC_Cabling_entries.cxx

    LINK_LIBRARIES
    AthenaBaseComps
    AthenaKernel
    AthenaPoolUtilities
    GaudiKernel
    StoreGateLib
    MuonCablingData
    MuonIdHelpersLib
)

atlas_install_python_modules(
    python/*.py
)

