# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigInDetR3PattRecoTools )

# library in the package:
atlas_add_library( TrigInDetR3PattRecoToolsLib
                   src/TrigInDetR3Utils.cxx
                   PUBLIC_HEADERS TrigInDetR3PattRecoTools
                   LINK_LIBRARIES TrigInDetPattRecoEvent
                   PRIVATE_LINK_LIBRARIES IRegionSelector TrigInDetEvent InDetPrepRawData BeamSpotConditionsData)

# Component(s) in the package:
atlas_add_component( TrigInDetR3PattRecoToolsComp
                     src/SeedingToolBase.cxx
                     src/TrigInDetR3TrackSeedingTool.cxx
		     src/GNNR3_*.cxx
                     src/components/*.cxx
		     LINK_LIBRARIES AthenaBaseComps AtlasDetDescr GaudiKernel PathResolver InDetIdentifier InDetPrepRawData MagFieldConditions MagFieldElements StoreGateLib InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry TrigInDetEvent TrigInDetPattRecoEvent TrigInDetToolInterfacesLib TrkEventPrimitives TrkParameters TrkPrepRawData TrkRIO_OnTrack TrkSurfaces TrkToolInterfaces TrkTrack IRegionSelector BeamSpotConditionsData ActsToolInterfacesLib TrigAccelEvent TrigInDetAccelerationServiceLib)


