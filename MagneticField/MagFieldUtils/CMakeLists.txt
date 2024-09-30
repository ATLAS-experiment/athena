# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MagFieldUtils )

# External dependencies:
find_package( ROOT COMPONENTS RIO Core Tree)

atlas_add_library(MagFieldUtilsLib
                  Root/*.cxx
                  PUBLIC_HEADERS MagFieldUtils
                  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                  LINK_LIBRARIES ${ROOT_LIBRARIES} MagFieldElements)

# Component(s) in the package:
atlas_add_component(MagFieldUtils
                   src/*.cxx
                   src/components/*.cxx
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel MagFieldConditions StoreGateLib )

