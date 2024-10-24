# Copyright (C) 2002-2024 CERN for thple benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkStripCabling )

find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( ITkStripCablingLib
                   src/ITkStripOnlineId.cxx src/ITkStripCablingData.cxx
                   PUBLIC_HEADERS ITkStripCabling
                   LINK_LIBRARIES AthenaKernel Identifier
                   PRIVATE_LINK_LIBRARIES InDetIdentifier )

atlas_add_component( ITkStripCabling
                     src/ITkStripCablingAlg.cxx src/components/*.cxx 
                     LINK_LIBRARIES ITkStripCablingLib 
                     PRIVATE_LINK_LIBRARIES AthenaBaseComps InDetIdentifier PathResolver IdDictParser StoreGateLib)


                
atlas_add_test( ITkStripOnlineId_test	
  SOURCES 
  test/ITkStripOnlineId_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils ITkStripCablingLib
  POST_EXEC_SCRIPT nopost.sh
)


atlas_add_test( ITkStripCablingData_test	
  SOURCES 
  test/ITkStripCablingData_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils ITkStripCablingLib
  POST_EXEC_SCRIPT nopost.sh
)



atlas_add_test( OnlineIdGenerator_test	
  SOURCES 
  src/OnlineIdGenerator.cxx  test/OnlineIdGenerator_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps  
    PathResolver StoreGateLib IdDictParser ITkStripCablingLib 
    InDetIdentifier
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( ITkStripCablingAlg_test	
  SOURCES 
  src/ITkStripCablingAlg.cxx src/OnlineIdGenerator.cxx src/components/*.cxx test/ITkStripCablingAlg_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps  
    PathResolver StoreGateLib IdDictParser ITkStripCablingLib 
    InDetIdentifier
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_executable( ProduceDummyStripCabling
  SOURCES src/OnlineIdGenerator.cxx utilities/ProduceDummyStripCabling.cxx 
  LINK_LIBRARIES   IdDictParser ITkStripCablingLib  TestTools  InDetIdentifier 
)

atlas_install_joboptions(share/*.txt)
atlas_install_runtime(share/*.dat)




