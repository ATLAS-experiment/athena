# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FPGATrkConverter )

# Component(s) in the package:
atlas_add_component( FPGATrkConverter
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES ActsEventLib ActsGeometryLib AthenaBaseComps AtlasDetDescr
                     BeamSpotConditionsData FPGATrackSimObjectsLib FPGATrkConverterInterface
                     GaudiKernel Identifier InDetCondTools InDetIdentifier InDetPrepRawData
                     PixelReadoutGeometryLib SCT_ReadoutGeometry StoreGateLib TrkEventPrimitives
                     xAODInDetMeasurement )
