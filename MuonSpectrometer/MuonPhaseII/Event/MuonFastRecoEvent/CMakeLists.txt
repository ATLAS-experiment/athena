# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonFastRecoEvent )


find_package( Acts COMPONENTS Core )

atlas_add_library( MuonFastRecoEvent
                   src/*.cxx 
                   PUBLIC_HEADERS MuonFastRecoEvent
                   LINK_LIBRARIES xAODMuonPrepData Acts::Core Identifier MuonReadoutGeometryR4 xAODMeasurementBase 
                                  MuonDetDescrUtils xAODMuon MuonSpacePoint MuonStationIndexLib)