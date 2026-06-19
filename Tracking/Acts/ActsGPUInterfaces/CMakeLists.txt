# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUInterfaces )

find_package(Acts)

# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()

find_package(traccc)

atlas_add_library( ActsGPUInterfacesLib
                   INTERFACE
                   PUBLIC_HEADERS ActsGPUInterfaces
                   LINK_LIBRARIES
                   GaudiKernel
                   Identifier
                   traccc::core
                 )