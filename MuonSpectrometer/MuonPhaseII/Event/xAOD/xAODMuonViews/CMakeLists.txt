# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonViews )


find_package(Acts CONFIG COMPONENTS Core)

atlas_add_library( xAODMuonViews
    xAODMuonViews/*.h
    INTERFACE
    PUBLIC_HEADERS xAODMuonViews
    LINK_LIBRARIES xAODMeasurementBase Identifier MuonIdHelpersLib Acts::Core)


   
