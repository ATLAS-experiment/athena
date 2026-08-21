# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name
atlas_subdir( HGTD_AlignGenTools )

# External dependencies
find_package( CLHEP )

# Library
atlas_add_library(
    HGTD_AlignGenToolsLib
    HGTD_AlignGenTools/*.h
    INTERFACE
    PUBLIC_HEADERS HGTD_AlignGenTools
    LINK_LIBRARIES
        AthenaBaseComps
        AthenaKernel
        AthenaPoolUtilities
        GaudiKernel
        GeoPrimitives
        Identifier
        StoreGateLib
)

# Components
atlas_add_component(
    HGTD_AlignGenTools
    src/*.cxx
    src/components/*.cxx

    INCLUDE_DIRS
        ${CLHEP_INCLUDE_DIRS}

    LINK_LIBRARIES

        ${CLHEP_LIBRARIES}

        AthenaBaseComps
        AthenaKernel
        AthenaPoolUtilities

        GaudiKernel
        GeoPrimitives

        Identifier
        StoreGateLib

        AtlasDetDescr
        DetDescrConditions

        HGTD_ReadoutGeometry
        HGTD_AlignGenToolsLib

        RegistrationServicesLib
)

# Python configuration
atlas_install_python_modules(
    python/*.py
    POST_BUILD_CMD ${ATLAS_FLAKE8}
)