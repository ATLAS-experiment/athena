# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SiSPSeededTrackFinderData )

# Component(s) in the package:
atlas_add_library( SiSPSeededTrackFinderData
                   src/*.cxx
                   PUBLIC_HEADERS SiSPSeededTrackFinderData
                   LINK_LIBRARIES AthenaKernel InDetPrepRawData InDetReadoutGeometry SiSpacePointsSeed TrkEventPrimitives TrkExInterfaces TrkGeometry TrkPatternParameters TrkSurfaces TrkTrack TrkCaloClusterROI TrkToolInterfaces TrkEventUtils MagFieldElements MagFieldConditions InDetConditionsSummaryService xAODInDetMeasurement SiSpacePoint
                   PRIVATE_LINK_LIBRARIES InDetRIO_OnTrack StoreGateLib TrkExUtils TrkMaterialOnTrack TrkPrepRawData TrkRIO_OnTrack TrkSpacePoint)
