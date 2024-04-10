# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonAGDD )

# Possible extra dependencies:
set( extra_lib )
if( NOT SIMULATIONBASE )
   # the dependency on AmdcAth is only for dumping the passive material XML 
   # located inside old AMDB files (using AmdcsimrecAthenaSvc in the MuonAGDDTool)
   # in principle, AGDD is independent from AMDB and all information can be accessed 
   # via AGDD itself (as it is done in recent muon layouts)
   set( extra_lib AmdcAthLib )
endif()

# Component(s) in the package:
atlas_add_component( MuonAGDD
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES AGDDControl GaudiKernel StoreGateLib AGDDKernel AGDDModel GeoModelInterfaces MuonAGDDBase MuonReadoutGeometry MuonDetDescrUtils RDBAccessSvcLib ${extra_lib} )

