# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: MuonVisualizationHelpersR4
################################################################################

atlas_subdir (MuonVisualizationHelpersR4)


find_package( ROOT COMPONENTS Graf Core )



atlas_add_library( MuonVisualizationHelpersR4
                   src/*.cxx
                  PUBLIC_HEADERS MuonVisualizationHelpersR4
                  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                  LINK_LIBRARIES ${ROOT_LIBRARIES} MuonPatternEvent MuonSpacePoint GeoPrimitives)
              

