# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigBSExtraction )

# Component(s) in the package:
atlas_add_component( TrigBSExtraction
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib TrigNavigationLib TrigSerializeCnvSvcLib TrigSteeringEvent )
