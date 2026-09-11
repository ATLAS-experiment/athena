# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CollectionSvc )

# External dependencies:
find_package( CORAL COMPONENTS CoralBase )

# Component(s) in the package:
atlas_add_library( CollectionSvc
                   src/*.cpp
                   PUBLIC_HEADERS CollectionSvc
                   INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${CORAL_LIBRARIES} CxxUtils StorageSvc
                   PRIVATE_LINK_LIBRARIES AthenaKernel GaudiKernel
                      PersistentDataModel PoolSvcLib )

atlas_add_dictionary( CollectionSvcDict
                      CollectionSvc/CollectionSvcDict.h
                      CollectionSvc/selection.xml
                      INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
                      LINK_LIBRARIES CollectionSvc )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if( NOT GENERATIONBASE )
  atlas_add_test( collSorting_test
                  SCRIPT test/test_CollSort.py
                  POST_EXEC_SCRIPT noerror.sh )

  atlas_add_test( ttree_coll_rw_test
                  SOURCES test/ttree_rw_test.cxx test/rw_test.cxx
                  LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionSvc PersistentDataModel StorageSvc CxxUtils AthenaKernel
                  LOG_IGNORE_PATTERN "POOLCollectionID.*Value=" )

  atlas_add_test( rntuple_coll_rw_test
                  SOURCES test/rntuple_rw_test.cxx test/rw_test.cxx
                  LINK_LIBRARIES ${CORAL_LIBRARIES} CollectionSvc PersistentDataModel StorageSvc CxxUtils AthenaKernel
                  LOG_IGNORE_PATTERN "POOLCollectionID.*Value=" )

  # wait for the test collections to be created, then sort them
  atlas_add_test( sort_test
                  SCRIPT test/sort_test.py
                  DEPENDS ttree_coll_rw_test rntuple_coll_rw_test )

endif()
