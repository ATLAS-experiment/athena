atlas_subdir( MuonR4TGC_Cabling )

atlas_add_component(
    MuonR4TGC_Cabling
    src/TgcCablingAlg.cxx
    src/components/MuonR4TGC_Cabling_entries.cxx

    LINK_LIBRARIES
    AthenaBaseComps
    AthenaKernel
    AthenaPoolUtilities
    GaudiKernel
    StoreGateLib
    MuonCablingDataR4
    MuonIdHelpersLib
)

atlas_add_test(TgcCablingAlg_test
    SCRIPT python ${CMAKE_CURRENT_SOURCE_DIR}/test/TgcCablingAlg_test.py
    PROPERTIES TIMEOUT 300
)

atlas_install_python_modules(
    python/*.py
)

