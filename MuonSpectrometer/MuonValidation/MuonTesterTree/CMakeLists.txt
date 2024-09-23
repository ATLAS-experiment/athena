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
                    LINK_LIBRARIES ${ROOT_LIBRARIES} xAODEventInfo xAODMuon xAODTruth xAODTracking  AthenaKernel StoreGateLib AthenaBaseComps AtlasHepMCLib ${extra_libs})
endif()
 
