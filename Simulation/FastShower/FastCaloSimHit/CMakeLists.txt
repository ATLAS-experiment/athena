# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( FastCaloSimHit )

# Component(s) in the package:

atlas_add_component( FastCaloSimHit
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel CaloInterfaceLib LArSimEvent TileSimEvent StoreGateLib PileUpToolsLib TileConditionsLib CaloEvent CaloIdentifier CxxUtils EventInfo TileEvent )
