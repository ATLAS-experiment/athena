# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsObjectDecoration)

# External dependencies:
find_package(Acts COMPONENTS Core)

atlas_add_component( ActsObjectDecoration
   src/*.h src/*.cxx src/detail/*.h src/detail/*.cxx
   src/components/*.cxx
   LINK_LIBRARIES
   ActsCore
   ActsEventLib
   ActsGeometryLib
   xAODInDetMeasurement
   xAODMeasurementBase
   InDetReadoutGeometry
   TrkTruthData
   InDetSimData
   InDetSimEvent
   PixelReadoutGeometryLib
   InDetMeasurementUtilitiesLib
)
