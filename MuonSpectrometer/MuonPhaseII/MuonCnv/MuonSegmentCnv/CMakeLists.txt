# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonSegmentCnv )

# Component(s) in the package:
atlas_add_component( MuonSegmentCnv
                     src/*.cxx
                     src/components/*.cxx                  
                     LINK_LIBRARIES GaudiKernel AthenaBaseComps StoreGateLib MuonPatternEvent MuonPrepRawData xAODMuon ActsEventLib
                                    MuonRecToolInterfaces MuonIdHelpersLib TrkSegment TrkExInterfaces MuonSegment xAODMuonPrepData
                                    MuonCompetingRIOsOnTrack TrkSurfaces MuonRecHelperToolsLib MuonReadoutGeometryR4)

atlas_install_python_modules( python/*.py)