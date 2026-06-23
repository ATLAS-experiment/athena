# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkVertexFitterInterfaces )


# Component(s) in the package:
atlas_add_library( TrkVertexFitterInterfaces
                   PUBLIC_HEADERS TrkVertexFitterInterfaces
                   LINK_LIBRARIES GeoPrimitives EventPrimitives xAODTracking GaudiKernel TrkNeutralParameters TrkParameters )
