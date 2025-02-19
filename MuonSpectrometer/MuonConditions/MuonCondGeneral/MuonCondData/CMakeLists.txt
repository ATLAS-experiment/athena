# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonCondData )

# External dependencies:
find_package( CLHEP )
find_package( GeoModel COMPONENTS GeoModelHelpers )

# Component(s) in the package:
atlas_add_library( MuonCondData
                   src/*.cxx
                   PUBLIC_HEADERS MuonCondData
                   PRIVATE_INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${GEOMODEL_INCLUDE_DIRS}
                   LINK_LIBRARIES AthContainers AthenaBaseComps AthenaKernel AthenaPoolUtilities GaudiKernel
                                  GeoModelUtilities GeoPrimitives Identifier MuonIdHelpersLib MuonReadoutGeometry
                   PRIVATE_LINK_LIBRARIES ${CLHEP_LIBRARIES} ${GEOMODEL_LIBRARIES} MuonNSWCommonDecode )
