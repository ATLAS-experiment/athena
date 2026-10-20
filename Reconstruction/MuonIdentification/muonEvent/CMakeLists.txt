# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( muonEvent )

# Component(s) in the package:
atlas_add_library( muonEvent
                   src/*.cxx
                   PUBLIC_HEADERS muonEvent
                   LINK_LIBRARIES AthContainers AthenaKernel CaloIdentifier Navigation TrkMaterialOnTrack
                   PRIVATE_LINK_LIBRARIES GaudiKernel )

atlas_add_dictionary( muonEventDict
                      muonEvent/muonEventDict.h
                      muonEvent/selection.xml
                      LINK_LIBRARIES Navigation muonEvent
                      ELEMENT_LINKS MuonCaloEnergyContainer )

