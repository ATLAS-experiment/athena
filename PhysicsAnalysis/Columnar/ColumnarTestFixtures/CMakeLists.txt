# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#
# @author Nils Krumnack

atlas_subdir( ColumnarTestFixtures )

find_package( GTest )
find_package( nlohmann_json )

set (extra_libs)
if (XAOD_STANDALONE)
  set (extra_libs )
else()
  set (extra_libs POOLRootAccessLib)
endif ()

# Add the shared library:
atlas_add_library (ColumnarTestFixturesLib
ColumnarTestFixtures/*.h Root/*.cxx
  PUBLIC_HEADERS ColumnarTestFixtures
  INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
  LINK_LIBRARIES ${GTEST_LIBRARIES} ColumnarToolWrapperLib PATInterfaces AsgTestingLib xAODJet xAODMissingET
  PRIVATE_LINK_LIBRARIES ${extra_libs} ColumnarCoreLib nlohmann_json::nlohmann_json xAODCaloEvent xAODTracking xAODMuon xAODEventInfo xAODEgamma xAODRootAccess)
