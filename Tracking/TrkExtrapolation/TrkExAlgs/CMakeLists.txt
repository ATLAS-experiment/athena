# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkExAlgs )

# External dependencies:
find_package( CLHEP )
find_package( ROOT COMPONENTS Core Tree RIO )
find_package( Acts COMPONENTS Core )

# Component(s) in the package:
atlas_add_component( TrkExAlgs
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                     LINK_LIBRARIES ${CLHEP_LIBRARIES} ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel AthContainers GeoPrimitives EventPrimitives GaudiKernel TrkSurfaces TrkParameters TrkParametersBase TrkGeometry TrkVolumes TrkEventPrimitives TrkTrack TrkExInterfaces ActsGeometryInterfacesLib ActsInteropLib ActsGeometryLib Acts::Core )
