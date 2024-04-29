# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ActsGeantFollowing )

# External dependencies:
find_package( Acts COMPONENTS Core )
find_package( Geant4 )
find_package( ROOT COMPONENTS Core Tree RIO )

atlas_add_library( ActsGeantFollowing
                   OBJECT
                   NO_PUBLIC_HEADERS
                     src/*.cxx
                     src/components/*.cxx
                     PRIVATE_INCLUDE_DIRS  ${ROOT_INCLUDE_DIRS} ${GEANT4_INCLUDE_DIRS}
                     PRIVATE_LINK_LIBRARIES  ${ROOT_LIBRARIES} ${GEANT4_LIBRARIES}
                     ActsCore
                     ActsGeometryInterfacesLib
                     ActsGeometryLib
                     ActsInteropLib
                     EventPrimitives
                     GeoPrimitives
                     AthenaBaseComps
                     AthenaKernel
                     CaloDetDescrLib
                     EventInfo
                     G4AtlasInterfaces 
                     G4AtlasToolsLib
                     GaudiKernel
                     StoreGateLib
                     TRT_ReadoutGeometry
                     TrkGeometry
                     TrkExInterfaces
                     TrkParameters
                     TrkSurfaces)       

# Install files from the package:
atlas_install_joboptions( share/*.py )
