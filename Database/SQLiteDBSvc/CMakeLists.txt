# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SQLiteDBSvc )

# External dependencies:
find_package( SQLite3 )

# Component(s) in the package:
atlas_add_library( SQLiteDBSvcLib
   src/Statement.cxx
   PUBLIC_HEADERS SQLiteDBSvc
   INCLUDE_DIRS ${SQLite3_INCLUDE_DIRS}
   LINK_LIBRARIES GaudiKernel ${SQLite3_LIBRARIES} )


atlas_add_component( SQLiteDBSvc
   src/SQLiteDBSvc.cxx src/components/*.cxx
   LINK_LIBRARIES GaudiKernel AthenaBaseComps AthenaKernel 
   SQLiteDBSvcLib )


# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8})
atlas_install_joboptions( share/*.txt )

# Test(s) in the package.
atlas_add_test( SQLiteDBSvc_test
   SOURCES test/SQLiteDBSvc_test.cxx
   LINK_LIBRARIES CxxUtils SQLiteDBSvcLib TestTools )
