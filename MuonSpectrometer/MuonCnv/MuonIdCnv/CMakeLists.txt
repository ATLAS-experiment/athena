# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonIdCnv )

# Component(s) in the package:
atlas_add_component( MuonIdCnv
                     src/*.cxx
                     src/*.h
                     src/components/*.cxx
                     LINK_LIBRARIES StoreGateLib DetDescrCnvSvcLib IdDictDetDescr GaudiKernel MuonIdHelpersLib AthenaKernel )

