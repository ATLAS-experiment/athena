# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name
atlas_subdir( HGTD_DetDescrAlgs )

# External dependencies
find_package( CLHEP )

# Component(s)
atlas_add_component( HGTD_DetDescrAlgs
                     src/*.cxx
                     src/components/*.cxx

                     INCLUDE_DIRS
                     ${CLHEP_INCLUDE_DIRS}

                     LINK_LIBRARIES
                     ${CLHEP_LIBRARIES}

                     AthenaBaseComps
                     AthenaKernel
                     StoreGateLib
                     GaudiKernel

                     Identifier
                     GeoPrimitives

                     HGTD_Identifier
                     HGTD_ReadoutGeometry
                     GeoModelUtilities
)

# Install scripts
atlas_install_scripts(
    tools/HGTD_RunPrintElements.py
    POST_BUILD_CMD ${ATLAS_FLAKE8}
)