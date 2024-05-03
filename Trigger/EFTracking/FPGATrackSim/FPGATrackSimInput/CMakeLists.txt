# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

atlas_subdir( FPGATrackSimInput )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( FPGATrackSimInputLib
   src/*.cxx
   PUBLIC_HEADERS            FPGATrackSimInput
   INCLUDE_DIRS              ${ROOT_INCLUDE_DIRS}
   LINK_LIBRARIES            ${ROOT_LIBRARIES} AthenaBaseComps GaudiKernel InDetReadoutGeometry
                             FPGATrackSimConfToolsLib FPGATrackSimMapsLib FPGATrackSimObjectsLib
   PRIVATE_LINK_LIBRARIES    AthContainers InDetIdentifier  StoreGateLib FPGATrackSimSGInputLib )

atlas_add_component( FPGATrackSimInput
   src/components/*.cxx
   LINK_LIBRARIES            FPGATrackSimInputLib
)

# Install files from the package:
atlas_install_scripts( scripts/*)
