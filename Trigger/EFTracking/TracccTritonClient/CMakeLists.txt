# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TracccTritonClient )

find_package( Acts COMPONENTS Core )

if (NOT TARGET Acts::PluginDetray)
   return()
endif()

find_package(vecmem)
find_package(traccc)

# Header-only (de)serialization of traccc EDM collections, shared with the
# TracccTritonBackend server
atlas_add_library( TracccTritonClientLib
    TracccTritonClient/*.h
    INTERFACE
    PUBLIC_HEADERS TracccTritonClient
    LINK_LIBRARIES vecmem::core )

# Component(s) in the package:
atlas_add_component( TracccTritonClient
    src/*.cxx
    src/components/*.cxx
    LINK_LIBRARIES
        AthenaBaseComps AthenaKernel GaudiKernel StoreGateLib
        AthOnnxInterfaces AthDeviceInterfacesLib
        vecmem::core traccc::core
        ActsGPUEventLib TracccTritonClientLib
)

atlas_install_python_modules( python/*.py )
