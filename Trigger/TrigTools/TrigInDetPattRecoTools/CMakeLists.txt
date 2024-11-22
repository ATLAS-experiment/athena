# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigInDetPattRecoTools )

# library in the package:
atlas_add_library( TrigInDetPattRecoToolsLib
                   src/TrigInDetUtils.cxx
		   src/TrigTrackSeedGenerator.cxx
                   PUBLIC_HEADERS TrigInDetPattRecoTools
                   LINK_LIBRARIES TrigInDetPattRecoEvent
                   PRIVATE_LINK_LIBRARIES IRegionSelector TrigInDetEvent InDetPrepRawData BeamSpotConditionsData)

# Component(s) in the package:
atlas_add_component( TrigInDetPattRecoToolsComp
                     src/TrigInDetTrackSeedingTool.cxx
		     src/GNN_*.cxx
                     src/components/*.cxx
		     LINK_LIBRARIES AthenaBaseComps AtlasDetDescr GaudiKernel PathResolver InDetIdentifier InDetPrepRawData MagFieldConditions MagFieldElements StoreGateLib InDetReadoutGeometry PixelReadoutGeometryLib SCT_ReadoutGeometry TrigInDetEvent TrigInDetPattRecoEvent TrigInDetToolInterfacesLib TrkEventPrimitives TrkParameters TrkPrepRawData TrkRIO_OnTrack TrkSurfaces TrkToolInterfaces TrkTrack IRegionSelector BeamSpotConditionsData )

atlas_add_test( mockSeeds
                SOURCES test/mockSeeds.cxx
                LINK_LIBRARIES PathResolver TrigInDetEvent TrigInDetPattRecoEvent TrigInDetPattRecoToolsLib TrigSteeringEvent IRegionSelector
                POST_EXEC_SCRIPT nopost.sh
                PROPERTIES TIMEOUT 300 )


