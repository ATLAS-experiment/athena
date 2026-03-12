# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrackingDecorAlgorithms )

if( NOT XAOD_STANDALONE )
  atlas_add_component( TrackingDecorAlgorithms
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES AthenaBaseComps StoreGateLib xAODTracking )
endif()
