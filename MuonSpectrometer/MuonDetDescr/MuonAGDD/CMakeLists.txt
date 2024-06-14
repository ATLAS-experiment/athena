# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonAGDD )


# Component(s) in the package:
atlas_add_component( MuonAGDD
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AGDDControl GaudiKernel StoreGateLib AGDDKernel AGDDModel GeoModelInterfaces 
                                   MuonAGDDBase MuonReadoutGeometry MuonDetDescrUtils RDBAccessSvcLib )

