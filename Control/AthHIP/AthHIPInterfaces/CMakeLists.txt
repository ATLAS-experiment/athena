# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthHIPInterfaces)

# Bail out if ROCm/HIP is not available.
if(NOT CMAKE_HIP_COMPILER)
   message(STATUS "ROCm/HIP not available, not building package")
   return()
endif()

# Package dependencies.
find_package(hip)

# Component(s) in the package.
atlas_add_library(AthHIPInterfacesLib
   AthHIPInterfaces/*.h
   INTERFACE
   PUBLIC_HEADERS AthHIPInterfaces
   LINK_LIBRARIES hip::host GaudiKernel)
