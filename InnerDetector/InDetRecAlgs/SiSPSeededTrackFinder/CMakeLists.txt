# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SiSPSeededTrackFinder )

# Component(s) in the package:
atlas_add_component( SiSPSeededTrackFinder
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps StoreGateLib GaudiKernel BeamSpotConditionsData InDetRecToolInterfaces IRegionSelector RoiDescriptor TrkCaloClusterROI TrkGeometry TrkSurfaces TrkSpacePoint TrkTrack TrkExInterfaces xAODEventInfo SiSPSeededTrackFinderData TrkPatternParameters TrkRIO_OnTrack TrkEventUtils TrkToolInterfaces xAODTracking)
