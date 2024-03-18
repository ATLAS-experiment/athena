# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigInDetTrackFitter )

# Component(s) in the package:
atlas_add_component( TrigInDetTrackFitter
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps AtlasDetDescr GaudiKernel InDetIdentifier InDetPrepRawData InDetRIO_OnTrack MagFieldConditions MagFieldElements StoreGateLib InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry TrigInDetEvent TrigInDetToolInterfacesLib TrkDistributedKalmanFilterLib TrkEventPrimitives TrkParameters TrkPrepRawData TrkRIO_OnTrack TrkSurfaces TrkToolInterfaces TrkTrack )
