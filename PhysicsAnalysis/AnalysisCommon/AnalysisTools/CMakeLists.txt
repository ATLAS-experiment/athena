# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( AnalysisTools )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( AthAnalysisToolsLib
                   src/*.cxx
                   PUBLIC_HEADERS AnalysisTools
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AnalysisUtilsLib AthenaBaseComps CxxUtils GaudiKernel
                   PRIVATE_LINK_LIBRARIES ${CORAL_LIBRARIES} AthenaPoolUtilities EventInfo PersistentDataModel SGTools StoreGateLib xAODEventInfo )

atlas_add_component( AthAnalysisTools
                     src/components/*.cxx
                     LINK_LIBRARIES AthAnalysisToolsLib )

