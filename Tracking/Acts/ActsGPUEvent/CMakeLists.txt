# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ActsGPUEvent )

find_package(Acts)

# placeholder for what will eventually be Acts::PluginTraccc
if (NOT TARGET Acts::PluginDetray)
   # Without PluginDetray, the traccc code will not be available.
   return()
endif()

find_package(traccc)

atlas_add_library( ActsGPUEventLib
                   src/ActsGPUEvent.cxx
                   PUBLIC_HEADERS ActsGPUEvent
                   LINK_LIBRARIES
                   AthenaKernel
                   traccc::core
                 )

atlas_generate_cliddb( ActsGPUEventLib )