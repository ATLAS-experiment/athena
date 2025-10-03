# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( DQUtils )

# External dependencies:
find_package( COOL COMPONENTS CoolKernel CoolApplication )
find_package( CORAL COMPONENTS CoralBase CoralKernel RelationalAccess )
find_package( Oracle )
find_package( Python COMPONENTS Development )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( sqlalchemy )

string(REPLACE "-Wl,--as-needed" "" CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

# Component(s) in the package:
atlas_add_library( DQUtils
                   src/quick_retrieve.cxx
                   src/pythonic_coracool.cxx
                   NO_PUBLIC_HEADERS
                   INCLUDE_DIRS ${ORACLE_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS}
                   PRIVATE_INCLUDE_DIRS ${COOL_INCLUDE_DIRS} ${Python_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ORACLE_LIBRARIES} ${ROOT_LIBRARIES}
                   PRIVATE_LINK_LIBRARIES ${COOL_LIBRARIES} ${Python_LIBRARIES} ${CORAL_LIBRARIES} CoraCool CxxUtils )

atlas_add_dictionary( DQUtilsDict
                      dict/dictionary.h
                      dict/dictionary.xml
                      INCLUDE_DIRS ${ORACLE_INCLUDE_DIRS} ${COOL_INCLUDE_DIRS} ${ROOT_INCLUDE_DIRS} ${Python_INCLUDE_DIRS} ${CORAL_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ORACLE_LIBRARIES} ${COOL_LIBRARIES} ${ROOT_LIBRARIES} ${Python_LIBRARIES} ${CORAL_LIBRARIES} CoraCool DQUtils )

# Install files from the package:
atlas_install_python_modules( python/*.py python/ext python/sugar
   POST_BUILD_CMD ${ATLAS_FLAKE8} --extend-select E11,E402,W1 --extend-ignore E731 )

foreach( _test coracool events iovset_pickle read iovtype oracle )
   atlas_add_test( ${_test} SCRIPT tests/test_${_test}.py POST_EXEC_SCRIPT nopost.sh )
endforeach()
