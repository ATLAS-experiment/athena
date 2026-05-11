# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir(AthDeviceInterfaces)

# Component(s) in the package.
atlas_add_library(AthDeviceInterfacesLib
   AthDeviceInterfaces/*.h
   INTERFACE
   PUBLIC_HEADERS AthDeviceInterfaces
   LINK_LIBRARIES GaudiKernel)
