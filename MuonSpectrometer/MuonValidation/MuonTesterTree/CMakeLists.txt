################################################################################
# Package: MuonTesterTree
################################################################################

# Declare the package name:
atlas_subdir( MuonTesterTree )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )


set(extra_libs)
if ( NOT XAOD_ANALYSIS )
  set(extra_libs MuonIdHelpersLib TrkEventPrimitives GeoPrimitives MuonSimEvent)
endif()
if ( NOT XAOD_STANDALONE)
    atlas_add_library( MuonTesterTreeLib
                    MuonTesterTree/*.h Root/*.cxx
                    PUBLIC_HEADERS MuonTesterTree
                    LINK_LIBRARIES ${ROOT_LIBRARIES} xAODEventInfo xAODMuon xAODTruth xAODTracking  AthenaKernel 
                                   StoreGateLib AthenaBaseComps AtlasHepMCLib ${extra_libs})
    atlas_add_component( MuonTesterTreeTest
                         src/components/*.cxx src/*.cxx
                        LINK_LIBRARIES AthenaKernel MuonTesterTreeLib GaudiKernel)

    if ( NOT XAOD_ANALYSIS )
        # Install files from the package:
        atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

        atlas_add_test( RunTesterTree
                        SCRIPT python -m MuonTesterTree.TesterTreeConfig                
                        PROPERTIES TIMEOUT 900
                        PRIVATE_WORKING_DIRECTORY
                        POST_EXEC_SCRIPT nopost.sh)
    endif()
endif()
 
