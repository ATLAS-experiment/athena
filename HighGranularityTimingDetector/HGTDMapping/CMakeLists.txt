# Declare the package name:
atlas_subdir( HGTDMapping )

find_package( ROOT COMPONENTS Core PyROOT Tree MathCore Hist RIO pthread )
find_package( Boost COMPONENTS unit_test_framework )

# Add the shared library:
atlas_add_library (HGTDMappingLib
            HGTDMapping/*.h src/*.cxx
            PUBLIC_HEADERS HGTDMapping
            INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
            LINK_LIBRARIES ${ROOT_LIBRARIES} PathResolver  AthenaBaseComps  CxxUtils GeoPrimitives Identifier HGTD_Identifier)

if (NOT XAOD_STANDALONE)

atlas_add_component ( HGTDMapping
            src/components/*.cxx
            LINK_LIBRARIES HGTDMappingLib
            GaudiKernel)
endif ()

# Install python modules, joboptions, and share content
atlas_install_joboptions(share/*.txt)
atlas_install_runtime(share/*.dat)

# Tests in the package:
atlas_add_test( HGTDOnlineID_test
  SOURCES
  test/HGTDOnlineID_test.cxx
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}
  LINK_LIBRARIES ${Boost_LIBRARIES} HGTDMappingLib
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( HGTDMappingData_test	
  SOURCES 
  test/HGTDMappingData_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} HGTDMappingLib
  POST_EXEC_SCRIPT nopost.sh
)

atlas_add_test( HGTDMappingAlg_test	
  SOURCES 
  src/HGTDMappingAlg.cxx src/components/*.cxx test/HGTDMappingAlg_test.cxx 
  INCLUDE_DIRS  ${Boost_INCLUDE_DIRS}  
  LINK_LIBRARIES ${Boost_LIBRARIES} TestTools AthenaBaseComps GaudiKernel
    PathResolver StoreGateLib Identifier IdDictParser HGTDMappingLib
    HGTD_Identifier
  POST_EXEC_SCRIPT nopost.sh
)
