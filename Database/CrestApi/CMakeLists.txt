
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Set the name of the package.
atlas_subdir( CrestApi )

# External dependencies.
find_package(nlohmann_json)
find_package(CURL)
find_package(Boost COMPONENTS system thread filesystem unit_test_framework regex timer)

set(SOURCES src/CrestApi.cxx src/CrestRequest.cxx src/CrestApiFs.cxx src/CrestModel.cxx src/CrestContainer.cxx)

# Component(s) in the package.
atlas_add_library( CrestApiLib
   ${SOURCES}
   PUBLIC_HEADERS CrestApi 
   LINK_LIBRARIES nlohmann_json::nlohmann_json
   PRIVATE_INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   PRIVATE_LINK_LIBRARIES ${CURL_LIBRARIES} ${Boost_LIBRARIES} )

# CrestApi package test for the file storage methods (CrestApiFs.cxx).
atlas_add_test( CrestApiFs_test
   SOURCES test/CrestApiFs_test.cxx
   LINK_LIBRARIES CrestApiLib nlohmann_json::nlohmann_json CxxUtils
   INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${CURL_LIBRARIES} ${Boost_LIBRARIES} stdc++
   POST_EXEC_SCRIPT nopost.sh )

# CrestApi package test for the server methods (CrestApi.cxx).
atlas_add_test( CrestApi_test
   SOURCES test/CrestApi_test.cxx
   LINK_LIBRARIES CrestApiLib nlohmann_json::nlohmann_json CxxUtils
   INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
   LINK_LIBRARIES ${CURL_LIBRARIES} ${Boost_LIBRARIES} stdc++
   POST_EXEC_SCRIPT nopost.sh )
