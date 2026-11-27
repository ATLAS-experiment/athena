# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( GeoModelSvc )

# External dependencies:
find_package( GeoModelCore )
find_package( GeoModelIO )

# Component(s) in the package:
atlas_add_component( GeoModelSvc
   src/*.h src/*.cxx src/components/*.cxx
   LINK_LIBRARIES GeoModelCore::GeoModelKernel GeoModelIO::GeoModelDBManager GeoModelIO::GeoModelRead
                  AthenaBaseComps AthenaKernel CxxUtils SGTools
                  StoreGateLib GeoModelInterfaces GeoModelUtilities GaudiKernel PathResolver
                  RDBAccessSvcLib EventInfoMgtLib )

