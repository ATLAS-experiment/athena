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
                   PRIVATE_LINK_LIBRARIES CxxUtils PersistencySvc )

atlas_add_dictionary( CollectionSvcDict
		CollectionSvc/CollectionSvcDict.h
		CollectionSvc/selection.xml
    INCLUDE_DIRS ${CORAL_INCLUDE_DIRS}
		LINK_LIBRARIES CollectionSvc )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

if( NOT GENERATIONBASE )
  atlas_add_test( Factory_test
                  SOURCES test/Factory_test.cxx
		  LINK_LIBRARIES CollectionSvc PersistentDataModel
                  PROPERTIES TIMEOUT 300 )

  atlas_add_test( collSorting_test
                  SCRIPT test/test_CollSort.py
                  POST_EXEC_SCRIPT noerror.sh )

endif()
