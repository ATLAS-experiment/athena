# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSegmentDriftCircleAssValidation )

# Component(s) in the package:
atlas_add_component( InDetSegmentDriftCircleAssValidation
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AtlasHepMCLib AthenaBaseComps GaudiKernel InDetPrepRawData TrkSegment TrkTruthData InDetRIO_OnTrack StoreGateLib TrkPseudoMeasurementOnTrack TrkTrack TruthUtils )
