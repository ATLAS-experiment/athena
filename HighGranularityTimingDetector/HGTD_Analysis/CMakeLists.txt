# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_Analysis )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO Physics )

# Component(s) in the package:
atlas_add_library( HGTD_AnalysisLib
                   src/*.cxx
                   PUBLIC_HEADERS HGTD_Analysis
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel xAODTracking xAODTruth
                   ${ROOT_LIBRARIES} )

atlas_add_component( HGTD_Analysis
                     src/components/*.cxx
                     LINK_LIBRARIES HGTD_AnalysisLib GaudiKernel )
