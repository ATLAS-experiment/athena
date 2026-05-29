################################################################################
# Package: IPPerformance
################################################################################

# Declare the package name:
atlas_subdir( IPPerformance )

# Find the needed external(s):
find_package( ROOT COMPONENTS Core RIO Hist Tree )

# build a shared library
atlas_add_library( IPPerformanceLib  
       	           IPPerformance/*.h
                   src/*.cxx
                   src/components/*.cxx          
                   ${IPPerformanceDictSource}
                   PUBLIC_HEADERS IPPerformance
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODBase xAODRootAccess
                   xAODEventInfo PileupReweightingLib PATInterfaces PATCoreLib
                   xAODJet
                   xAODTracking xAODTruth
                   JetCalibToolsLib
                   JetSelectorToolsLib AthContainers
                   AsgTools
                   JetUncertaintiesLib
                   TrigConfxAODLib
                   TrigDecisionToolLib xAODCutFlow JetMomentToolsLib
                   AnaAlgorithmLib EventBookkeeperToolsLib
		   TrackVertexAssociationToolLib InDetTrackSystematicsToolsLib 
                   ${release_libs}
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${extra_private_libs} AsgMessagingLib
)


atlas_add_component(IPPerformance
                      src/components/*.cxx
                      LINK_LIBRARIES IPPerformanceLib
  )


atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


