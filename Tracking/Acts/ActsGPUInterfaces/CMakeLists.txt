# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Set the package name.
atlas_subdir( ActsGPUInterfaces )

# External dependencies.
find_package(vecmem)

# Component(s) in the package.
atlas_add_library( ActsGPUInterfacesLib
                   INTERFACE
                   PUBLIC_HEADERS ActsGPUInterfaces
                   LINK_LIBRARIES
                   GaudiKernel
                   Identifier
                   vecmem::core
                 )
