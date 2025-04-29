# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CscClusterization )

# Component(s) in the package:
atlas_add_library( CscClusterizationLib
                   src/*.cxx
                   PUBLIC_HEADERS CscClusterization
                   LINK_LIBRARIES GaudiKernel Identifier MuonPrepRawData CscCalibToolsLib
                   PRIVATE_LINK_LIBRARIES AthenaBaseComps EventPrimitives MuonIdHelpersLib MuonRIO_OnTrack MuonReadoutGeometry TrkEventPrimitives TrkRIO_OnTrack TrkSurfaces CxxUtils )

atlas_add_component( CscClusterization
                     src/components/*.cxx
                     LINK_LIBRARIES CscClusterizationLib )
