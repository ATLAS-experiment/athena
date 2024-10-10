# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetTrackClusterAssValidation )

# Component(s) in the package:
atlas_add_component( InDetTrackClusterAssValidation
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AtlasHepMCLib AthenaBaseComps CxxUtils StoreGateLib InDetPrepRawData InDetReadoutGeometry TrkSpacePoint TrkTruthData GaudiKernel TrkRIO_OnTrack TrkTrack TruthUtils )
