# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

################################################################################
# Package: MuonReadoutGeometryR4
################################################################################

# Declare the package name:
atlas_subdir( MuonStationGeoHelpers )

atlas_add_library( MuonStationGeoHelpersLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonStationGeoHelpers
                   LINK_LIBRARIES AthenaBaseComps AthenaKernel GeoPrimitives Identifier GaudiKernel   MuonGeoModelR4Lib
                                  ActsGeometryInterfacesLib MuonReadoutGeometryR4 MuonIdHelpersLib 
                                  GeoModelUtilities CxxUtils)

atlas_add_component( MuonStationGeoHelpers
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES MuonStationGeoHelpersLib AthenaBaseComps AthenaKernel GeoPrimitives Identifier GaudiKernel  
                     ActsGeometryInterfacesLib  MuonReadoutGeometryR4 MuonIdHelpersLib 
                     GeoModelUtilities CxxUtils)

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
