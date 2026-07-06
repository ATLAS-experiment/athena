# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkFitterUtils )

# Component(s) in the package:
atlas_add_library( TrkFitterUtils
                   src/*.cxx
                   PUBLIC_HEADERS TrkFitterUtils
		           LINK_LIBRARIES AthContainers GeoPrimitives Identifier GaudiKernel TrkSurfaces TrkEventPrimitives TrkEventUtils TrkMeasurementBase TrkParameters TrkExUtils
		           PRIVATE_LINK_LIBRARIES AtlasDetDescr TrkMaterialOnTrack TrkPseudoMeasurementOnTrack TrkRIO_OnTrack TrkSegment TrkTrack TrkVertexOnTrack TrkExInterfaces TrkToolInterfaces )
