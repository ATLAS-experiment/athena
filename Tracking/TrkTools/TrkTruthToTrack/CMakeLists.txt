# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrkTruthToTrack )

# Component(s) in the package:
atlas_add_component( TrkTruthToTrack
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AtlasHepMCLib AthenaBaseComps GaudiKernel StoreGateLib TrackRecordLib TrkParameters TrkToolInterfaces xAODTruth TrkExInterfaces TruthUtils )
