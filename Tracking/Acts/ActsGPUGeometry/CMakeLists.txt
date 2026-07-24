# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUGeometry )

find_package(Acts)

# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()

find_package(traccc)
find_package(vecmem)

atlas_add_component( ActsGPUGeometry
  src/*.h src/*.cxx
  src/components/*.cxx

  LINK_LIBRARIES
    vecmem::core
    traccc::core
    traccc::io
    GaudiKernel
    AthenaBaseComps
    AthDeviceInterfacesLib
    PathResolver
    InDetIdentifier
    ActsGeometryInterfacesLib
    ActsGeometryLib
    ActsGPUEventLib
    ActsGPUInterfacesLib
    SiSpacePointFormationLib
    PixelReadoutGeometryLib
    SCT_ReadoutGeometry
)

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

# depends on having the configurations for EDM converters
# atlas_add_test( ActsDeviceGeometryTest
#     SCRIPT athena.py --threads=1 ${CMAKE_CURRENT_SOURCE_DIR}/test/ActsDeviceGeometryTest.py
#     POST_EXEC_SCRIPT nopost.sh
#     PROPERTIES SKIP_RETURN_CODE 2)