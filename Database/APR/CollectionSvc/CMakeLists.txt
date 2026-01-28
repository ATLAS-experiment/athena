# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CollectionSvc )

# External dependencies:
find_package( Boost )
find_package( CORAL COMPONENTS CoralBase )
find_package( XercesC )

# Component(s) in the package:
atlas_add_library( CollectionSvc
    src/*.cpp
		PUBLIC_HEADERS CollectionSvc
    INCLUDE_DIRS ${Boost_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
    LINK_LIBRARIES ${Boost_LIBRARIES} ${CORAL_LIBRARIES} FileCatalog CxxUtils
    PRIVATE_LINK_LIBRARIES CxxUtils POOLCore PersistencySvc StorageSvc )

atlas_add_component( CollectionSvcComponents
    src/components/*.cpp
    NO_PUBLIC_HEADERS
    LINK_LIBRARIES CollectionSvc )

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
                  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
		              LINK_LIBRARIES CollectionSvc POOLCore PersistentDataModel
                  ${ROOT_LIBRARIES}
                  PROPERTIES TIMEOUT 300 )

  atlas_add_test( collSorting_test
                  SCRIPT test/test_CollSort.py
                  POST_EXEC_SCRIPT nopost.sh )

endif()
