################################################################################
# Package: IPPerformance
################################################################################

# Declare the package name:
atlas_subdir( IPPerformance )

# Find the needed external(s):
find_package( ROOT COMPONENTS Core RIO Hist Tree )

#atlas_add_root_dictionary( IPPerformanceLib IPPerformanceDictSource
#	ROOT_HEADERS IPPerformance/*.h Root/LinkDef.h
#	EXTERNAL_PACKAGES ROOT)

if (XAOD_STANDALONE)
  set (extra_private_libs )
else ()
  set (extra_private_libs AthAnalysisBaseCompsLib)
endif ()


# build a shared library
#atlas_add_library( IPPerformanceLib IPPerformance/*.h Root/*.h Root/*.cxx ${IPPerformanceDictSource}
atlas_add_library( IPPerformanceLib  
       	           IPPerformance/*.h
                   Root/*.cxx
                   src/components/*.cxx          
                   ${IPPerformanceDictSource}
                   PUBLIC_HEADERS IPPerformance
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} xAODBase xAODRootAccess
                   xAODEventInfo GoodRunsListsLib PileupReweightingLib PATInterfaces PATCoreLib
                   PathResolver xAODTau xAODJet xAODMuon xAODEgamma
                   xAODTracking xAODTruth MuonMomentumCorrectionsLib
                   MuonEfficiencyCorrectionsLib MuonSelectorToolsLib JetCalibToolsLib
                   JetSelectorToolsLib AthContainers
                   ElectronPhotonFourMomentumCorrectionLib
                   ElectronEfficiencyCorrectionLib ElectronPhotonSelectorToolsLib
                   IsolationSelectionLib IsolationCorrectionsLib
                   FTagAnalysisInterfacesLib MuonAnalysisInterfacesLib
                   PhotonEfficiencyCorrectionLib METUtilitiesLib METInterface
                   TauAnalysisToolsLib AsgTools xAODMissingET
                   AssociationUtilsLib JetEDM JetUncertaintiesLib
                   JetCPInterfaces xAODBTaggingEfficiencyLib TrigConfxAODLib
                   TrigDecisionToolLib xAODCutFlow JetMomentToolsLib
                   TriggerMatchingToolLib xAODMetaData
                   JetJvtEfficiencyLib AnaAlgorithmLib PMGToolsLib JetSubStructureUtils JetSubStructureMomentToolsLib
		   TrackVertexAssociationToolLib InDetTrackSystematicsToolsLib 
                   ${release_libs}
                   PRIVATE_LINK_LIBRARIES ${Boost_LIBRARIES} ${extra_private_libs} AsgMessagingLib
)

if (XAOD_STANDALONE)
  atlas_add_dictionary (IPPerformanceDict
          IPPerformance/IPPerformanceDict.h
          IPPerformance/selection.xml
      	LINK_LIBRARIES ${ROOT_LIBRARIES} IPPerformanceLib xAODRootAccess 
          xAODEventInfo xAODTracking xAODJet xAODTruth GoodRunsListsLib TrigDecisionToolLib TrigConfxAODLib  
          InDetTrackSelectionToolLib xAODBTagging JetCalibToolsLib InDetTrackSystematicsToolsLib 
          TrackVertexAssociationToolLib JetSelectorToolsLib JetMomentToolsLib  PileupReweightingLib
          ${extra_libs}
  )
endif()

if( NOT XAOD_STANDALONE )
	atlas_add_component(IPPerformance
                      src/components/*.cxx
                      LINK_LIBRARIES IPPerformanceLib
  )
endif()


atlas_install_data( data/* )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_runtime( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )


