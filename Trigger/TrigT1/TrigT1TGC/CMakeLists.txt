# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigT1TGC )

# External dependencies:
find_package( ROOT COMPONENTS Core MathCore )

# Component(s) in the package:
atlas_add_library( TrigT1TGCLib
                   src/*.cxx
                   PUBLIC_HEADERS TrigT1TGC
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaBaseComps AthenaKernel ByteStreamData
                                  GaudiKernel Identifier MuonCondSvcLib MuonDigitContainer MuonRDO MuonTGC_CablingLib StoreGateLib TileEvent TrigT1Interfaces TGCTriggerCondSvcLib xAODMuon
		   PRIVATE_LINK_LIBRARIES AthenaPoolUtilities MuonCondInterface PathResolver TrigConfL1Data )

atlas_add_component( TrigT1TGC
                     src/components/*.cxx
                     LINK_LIBRARIES TrigT1TGCLib )

# Install files from the package:
atlas_install_runtime( share/TrigT1TGCMaskedChannel*.db )
