# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigKernel )

# Header-only boost dependency
find_package( Boost )

# Header-only library
atlas_add_library( TrigKernel
                   INTERFACE
                   PUBLIC_HEADERS TrigKernel
                   INCLUDE_DIRS ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES GaudiKernel )
