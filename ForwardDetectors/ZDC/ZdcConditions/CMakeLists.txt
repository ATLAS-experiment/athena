# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ZdcConditions )

# External dependencies:
find_package( nlohmann_json )

# Component(s) in the package:
atlas_add_library( ZdcConditions
                   src/*.cxx
                   PUBLIC_HEADERS ZdcConditions
                   LINK_LIBRARIES AthenaKernel Identifier ZdcIdentifier AsgMessagingLib nlohmann_json::nlohmann_json 
                   PRIVATE_LINK_LIBRARIES GaudiKernel RDBAccessSvcLib GeoModelInterfaces GeoModelUtilities GeoPrimitives GeometryDBSvcLib StoreGateLib PathResolver)

atlas_install_data( data/*.json )
