# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir(ActsGPUEventCnv)

# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()

find_package(vecmem)
find_package(traccc)
find_package(detray)

atlas_add_component( ActsGPUEventCnv
    src/*.h src/*.cxx
    src/components/*.cxx
    DEFINITIONS ALGEBRA_PLUGINS_INCLUDE_ARRAY
    LINK_LIBRARIES
    vecmem::core
    traccc::core
    traccc::io
    detray::io
    detray::core
    traccc::device_common
    GaudiKernel
    AthenaBaseComps
    ActsGeometryInterfacesLib
    ActsGPUInterfacesLib
    ActsGPUEventLib
    InDetRawData
    InDetPrepRawData
    InDetIdentifier
    xAODInDetMeasurement
    AthDeviceInterfacesLib
    PixelReadoutGeometryLib
    SCT_ReadoutGeometry
)
