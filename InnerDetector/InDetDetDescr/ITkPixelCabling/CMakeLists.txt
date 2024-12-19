# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ITkPixelCabling )

find_package(Boost COMPONENTS unit_test_framework)

# Component(s) in the package:
atlas_add_library( ITkPixelCablingLib
                   src/ITkPixelOnlineId.cxx src/ITkPixelCablingData.cxx
                   PUBLIC_HEADERS ITkPixelCabling
                   LINK_LIBRARIES AthenaKernel Identifier
                   PRIVATE_LINK_LIBRARIES InDetIdentifier )

atlas_add_component( ITkPixelCabling
                     src/ITkPixelCablingAlg.cxx src/components/*.cxx 
                     LINK_LIBRARIES ITkPixelCablingLib 
                     PRIVATE_LINK_LIBRARIES AthenaBaseComps InDetIdentifier PathResolver IdDictParser StoreGateLib)


                
atlas_add_test( ITkPixelOnlineId_test	
  SOURCES 
  test/ITkPixelOnlineId_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils ITkPixelCablingLib
  POST_EXEC_SCRIPT nopost.sh
)


atlas_add_test( ITkPixelCablingData_test	
  SOURCES 
  test/ITkPixelCablingData_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} CxxUtils ITkPixelCablingLib
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( ITkPixelCablingAlg_test	
  SOURCES 
  src/ITkPixelCablingAlg.cxx src/components/*.cxx test/ITkPixelCablingAlg_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps  
    PathResolver StoreGateLib IdDictParser ITkPixelCablingLib 
    InDetIdentifier
  POST_EXEC_SCRIPT nopost.sh
)
atlas_install_joboptions(share/*.txt)
atlas_install_runtime(share/*.dat)




