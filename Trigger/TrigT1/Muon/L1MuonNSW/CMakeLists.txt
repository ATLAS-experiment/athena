# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir ( L1MuonNSW )

# External dependencies
find_package( Acts COMPONENTS Core )

atlas_add_component( L1MuonNSW
        src/*.cxx src/components/*.cxx
        LINK_LIBRARIES Acts::Core
                       AthenaBaseComps
                       AthenaKernel
                       GaudiKernel
                       StoreGateLib
                       Identifier
                       GeoPrimitives
                       MuonIdHelpersLib
                       MuonDigitContainer
                       MuonReadoutGeometryR4
                       ActsGeometryInterfacesLib
                       xAODTrigL1Muon
                       AthenaMonitoringKernelLib )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-extensions=ATL901 )

atlas_add_test( L1MuonNSWConfig
                SCRIPT python -m L1MuonNSW.L1MuonNSWConfig
                PROPERTIES TIMEOUT 900
                POST_EXEC_SCRIPT noerror.sh )
