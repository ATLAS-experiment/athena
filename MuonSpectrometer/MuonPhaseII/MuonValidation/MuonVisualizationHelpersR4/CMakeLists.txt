# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
################################################################################
# Package: MuonVisualizationHelpersR4
################################################################################

atlas_subdir (MuonVisualizationHelpersR4)


find_package( ROOT COMPONENTS Gpad Graf Core Hist )



atlas_add_library( MuonVisualizationHelpersR4
                   Root/*.cxx
                  PUBLIC_HEADERS MuonVisualizationHelpersR4
                  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                  LINK_LIBRARIES ${ROOT_LIBRARIES} MuonPatternEvent MuonSpacePoint GeoPrimitives
                  PRIVATE_LINK_LIBRARIES MuonTrackEvent)
              

atlas_add_component( MuonVisualHelpersR4
                     src/components/*.cxx src/*.cxx
                     LINK_LIBRARIES AthenaKernel MuonVisualizationHelpersR4 MuonRecToolInterfacesR4)
