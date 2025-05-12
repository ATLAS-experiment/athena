# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( InDetSimEvent )

# External dependencies:
find_package( CLHEP )
find_package( Boost COMPONENTS unit_test_framework )

# Component(s) in the package:
atlas_add_library( InDetSimEvent
                   src/*.cxx
                   PUBLIC_HEADERS InDetSimEvent
                   INCLUDE_DIRS ${CLHEP_INCLUDE_DIRS}
                   DEFINITIONS ${CLHEP_DEFINITIONS}
                   LINK_LIBRARIES ${CLHEP_LIBRARIES} AthAllocators AthenaKernel GeneratorObjects HitManagement
                   PRIVATE_LINK_LIBRARIES GaudiKernel InDetIdentifier StoreGateLib )

atlas_add_dictionary( InDetSimEventDict
                      InDetSimEvent/InDetSimEventDict.h
                      InDetSimEvent/selection.xml
                      LINK_LIBRARIES InDetSimEvent )
                      
atlas_add_test( SiTotalCharge_test
  SOURCES test/SiTotalCharge_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools InDetSimEvent
  POST_EXEC_SCRIPT "nopost.sh"
)

atlas_add_test( SiCharge_test
  SOURCES test/SiCharge_test.cxx 
  INCLUDE_DIRS ${Boost_INCLUDE_DIRS} 
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools InDetSimEvent
  POST_EXEC_SCRIPT "nopost.sh"
)



