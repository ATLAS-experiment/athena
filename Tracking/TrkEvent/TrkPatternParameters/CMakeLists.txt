# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkPatternParameters )


# Component(s) in the package:
atlas_add_library( TrkPatternParameters
                   src/*.cxx
                   PUBLIC_HEADERS TrkPatternParameters
                   LINK_LIBRARIES TrkEventPrimitives TrkParameters
                   PRIVATE_LINK_LIBRARIES GaudiKernel TrkSurfaces )

