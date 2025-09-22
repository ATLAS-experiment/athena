# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RDBAccessSvc )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( SQLite3 )
find_package( Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( RDBAccessSvcLib
                   RDBAccessSvc/*.h
                   INTERFACE
                   PUBLIC_HEADERS RDBAccessSvc
                   INCLUDE_DIRS ${CORAL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CORAL_LIBRARIES} ${Boost_LIBRARIES} GaudiKernel DBLock )

atlas_add_component( RDBAccessSvc
                     src/*.cxx src/components/*.cxx
                     INCLUDE_DIRS ${SQLITE3_INCLUDE_DIRS}
                     LINK_LIBRARIES ${SQLITE3_LIBRARIES} AthenaBaseComps CxxUtils RDBAccessSvcLib )

atlas_add_executable( geomdb2sqlite
                      replicator/geomdb2sqlite.cpp
                      INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                      LINK_LIBRARIES ${CORAL_LIBRARIES} RDBAccessSvcLib )

atlas_add_executable( geomdb2oracle
                      replicator/geomdb2oracle.cpp
                      INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                      LINK_LIBRARIES ${CORAL_LIBRARIES} RDBAccessSvcLib CxxUtils )
                      
atlas_add_test( SqliteRecordset_test
  SOURCES src/SqliteRecord.cxx src/SqliteRecordset.cxx test/SqliteRecordset_test.cxx
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} ${SQLITE3_LIBRARIES} RDBAccessSvcLib AthenaBaseComps
  POST_EXEC_SCRIPT "nopost.sh"
)

# Install files from the package:
atlas_install_joboptions( share/*.py )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
