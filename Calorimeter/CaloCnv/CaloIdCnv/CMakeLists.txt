# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloIdCnv )

# Component(s) in the package:
atlas_add_component( CaloIdCnv
                     src/*.cxx src/components/*.cxx
                     LINK_LIBRARIES CaloIdentifier AthenaKernel DetDescrCnvSvcLib StoreGateLib IdDictDetDescr Identifier GaudiKernel )

