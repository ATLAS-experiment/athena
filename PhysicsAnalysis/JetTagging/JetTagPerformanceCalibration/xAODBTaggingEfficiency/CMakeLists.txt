# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODBTaggingEfficiency )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO MathMore Minuit Minuit2 Matrix Physics HistPainter Rint )
find_package( onnxruntime )

atlas_add_root_dictionary( xAODBTaggingEfficiencyLib xAODBTaggingEfficiencyLibDictSource ROOT_HEADERS xAODBTaggingEfficiency/TruthTagResults.h xAODBTaggingEfficiency/BTaggingToolUtil.h Root/LinkDef.h )

# Component(s) in the package:
atlas_add_library( xAODBTaggingEfficiencyLib
  ${xAODBTaggingEfficiencyLibDictSource}
  xAODBTaggingEfficiency/*.h Root/*.cxx
  PUBLIC_HEADERS xAODBTaggingEfficiency
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} AsgTools xAODBTagging xAODJet
  PATCoreLib PATInterfaces CalibrationDataInterfaceLib PathResolver FTagAnalysisInterfacesLib ${extra_libs} )


atlas_add_component( xAODBTaggingEfficiency
   src/components/*.cxx
   LINK_LIBRARIES xAODBTaggingEfficiencyLib )

atlas_add_dictionary( xAODBTaggingEfficiencyDict
  xAODBTaggingEfficiency/xAODBTaggingEfficiencyDict.h
  xAODBTaggingEfficiency/selection.xml
  LINK_LIBRARIES xAODBTaggingEfficiencyLib )

# Add tests scripts for Athena, AthAnalysis and AnalysisBase
atlas_add_executable(
  BTaggingToolsTester
  util/BTaggingToolsTester.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
)

atlas_add_executable(
  BTaggingEfficiencyToolTester
  util/BTaggingEfficiencyToolTester.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
)

atlas_add_executable(
  BTaggingJsonToolsTester
  util/BTaggingJsonToolsTester.cxx
  LINK_LIBRARIES xAODRootAccess AsgTools FTagAnalysisInterfacesLib
)

atlas_add_executable(
  BTaggingSelectionJsonInstantiationTester
  util/BTaggingSelectionJsonInstantiationTester.cxx
  LINK_LIBRARIES xAODRootAccess AsgTools FTagAnalysisInterfacesLib PathResolver
)

atlas_add_executable(
  BTaggingSystematicStrategyComparison
  util/BTaggingSystematicStrategyComparison.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib PathResolver
)

atlas_add_executable(
  BTaggingEigenVectorRecompositionToolTester
  util/BTaggingEigenVectorRecompositionToolTester.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
)

atlas_add_executable(
  BTaggingSelectionToolTester
  util/BTaggingSelectionToolTester.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
)

atlas_add_executable(
  BTaggingToolsExample
  util/BTaggingToolsExample.cxx
  INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
  LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
)

if( XAOD_STANDALONE )
  set(XAOD_ACCESS_LIB xAODRootAccess)
else()
  set(XAOD_ACCESS_LIB POOLRootAccessLib)
  atlas_add_executable( BTaggingTruthTaggingTester
    util/BTaggingTruthTaggingTester.cxx
    LINK_LIBRARIES AsgTools xAODBTaggingEfficiencyLib POOLRootAccessLib AthAnalysisBaseCompsLib xAODEventInfo xAODRootAccess AsgMessagingLib FTagAnalysisInterfacesLib )
endif()
target_link_libraries( BTaggingToolsTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingSelectionToolTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingToolsExample
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingEfficiencyToolTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingJsonToolsTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingSelectionJsonInstantiationTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingEigenVectorRecompositionToolTester
  PRIVATE ${XAOD_ACCESS_LIB} )
target_link_libraries( BTaggingSystematicStrategyComparison
  PRIVATE ${XAOD_ACCESS_LIB} )

unset(XAOD_ACCESS_LIB)

set(FTAG_INPUT_RUN3_DAOD /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7266/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p7266/DAOD_PHYS.49568713._001693.pool.root.1)
set(FTAG_INPUT_RUN2_DAOD /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p7266/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13145_r13146_p7266/DAOD_PHYS.49680245._000770.pool.root.1)
set(FTAG_LATEST_CDI_RUN2_PCBT xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root)
set(FTAG_LATEST_CDI_RUN2_PCFT xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root)
set(FTAG_LATEST_CDI_RUN3_PCBT xAODBTaggingEfficiency/13p6TeV/MC23_2026_05_13_GN2v01_v0.root)
set(FTAG_LATEST_CDI_RUN3_PCFT xAODBTaggingEfficiency/13p6TeV/MC23_2026_05_13_GN2v01_v0.root)
# Xbb and Xcc JSON files 
# For now files are on CVMFS dev space 
# /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/dev
# Hence using 
# LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file" 
set(FTAG_XBB_LATEST_JSON_RUN2And3 dev/xAODBTaggingEfficiency/13And13p6TeV/MC20And23_2026-08-05_GN3XPV01_HbbOnly_bJR10v01_noSFs_NominalMass_v00.json)
set(FTAG_XCC_LATEST_JSON_RUN2 dev/xAODBTaggingEfficiency/13TeV/MC20_2026-08-13_GN3XPV01_HccWithHbbVeto_bJR10v01_noSFs_v00.json)
set(FTAG_XCC_LATEST_JSON_RUN3 dev/xAODBTaggingEfficiency/13p6TeV/MC23_2026-08-13_GN3XPV01_HccWithHbbVeto_bJR10v01_noSFs_v00.json)

# Test for Athena, AthAnalysis and AnalysisBase

# Small-R jets CI tests
atlas_add_test(
  ut_BTaggingToolsExample
  SCRIPT BTaggingToolsExample ${FTAG_INPUT_RUN3_DAOD}
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingToolsTest
  SCRIPT BTaggingToolsTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 FixedCutBEff_77 1d
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingSelectionTool_continuous
  SCRIPT BTaggingSelectionToolTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCBT_Loose
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous Loose
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCBT_Medium
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous Medium
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCBT_Tight
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous Tight
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCBT_Loose
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCBT} GN2v01 Continuous Loose
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCBT_Medium
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCBT} GN2v01 Continuous Medium
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCBT_Tight
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCBT} GN2v01 Continuous Tight
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCFT_Loose
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCFT} GN2v01 Continuous2D Loose
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCFT_Medium
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCFT} GN2v01 Continuous2D Medium
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run2_PCFT_Tight
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN2_PCFT} GN2v01 Continuous2D Tight
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCFT_Loose
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCFT} GN2v01 Continuous2D Loose
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCFT_Medium
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCFT} GN2v01 Continuous2D Medium
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEigenVectorRecompositionToolTester_Run3_PCFT_Tight
  SCRIPT BTaggingEigenVectorRecompositionToolTester ${FTAG_LATEST_CDI_RUN3_PCFT} GN2v01 Continuous2D Tight
  POST_EXEC_SCRIPT noerror.sh
)
atlas_add_test(
  ut_BTaggingEfficiencyToolTester_Run3_PCBT
  SCRIPT BTaggingEfficiencyToolTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous
  REF_FILE share/ut_BTaggingEfficiencyToolTester_Run3_PCBT.ref
  LOG_IGNORE_PATTERN "^ApplicationMgr +INFO|^xAOD::Init +INFO|^xAOD::TEvent +WARNING"
)
atlas_add_test(
  ut_BTaggingEfficiencyToolTester_Run3_PCFT
  SCRIPT BTaggingEfficiencyToolTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_LATEST_CDI_RUN3_PCFT} GN2v01 Continuous2D
  REF_FILE share/ut_BTaggingEfficiencyToolTester_Run3_PCFT.ref
  LOG_IGNORE_PATTERN "^ApplicationMgr +INFO|^xAOD::Init +INFO|^xAOD::TEvent +WARNING"
)
atlas_add_test(
  ut_BTaggingEfficiencyToolTester_Run2_PCBT
  SCRIPT BTaggingEfficiencyToolTester ${FTAG_INPUT_RUN2_DAOD} ${FTAG_LATEST_CDI_RUN2_PCBT} GN2v01 Continuous
  REF_FILE share/ut_BTaggingEfficiencyToolTester_Run2_PCBT.ref
  LOG_IGNORE_PATTERN "^ApplicationMgr +INFO|^xAOD::Init +INFO|^xAOD::TEvent +WARNING"
)
atlas_add_test(
  ut_BTaggingEfficiencyToolTester_Run2_PCFT
  SCRIPT BTaggingEfficiencyToolTester ${FTAG_INPUT_RUN2_DAOD} ${FTAG_LATEST_CDI_RUN2_PCFT} GN2v01 Continuous2D
  REF_FILE share/ut_BTaggingEfficiencyToolTester_Run2_PCFT.ref
  LOG_IGNORE_PATTERN "^ApplicationMgr +INFO|^xAOD::Init +INFO|^xAOD::TEvent +WARNING"
)
atlas_add_test(
  ut_BTaggingSystematicStrategyComparison
  SCRIPT BTaggingSystematicStrategyComparison ${FTAG_INPUT_RUN3_DAOD} ${FTAG_LATEST_CDI_RUN3_PCBT} GN2v01 Continuous
  POST_EXEC_SCRIPT noerror.sh
)

# Large-R jet CI tests 
atlas_add_test(
  ut_BTaggingJsonToolsTest_Run2And3_GN3XPV01_hbb_bjr_FlatMassQCDEff_0p4
  SCRIPT BTaggingJsonToolsTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_XBB_LATEST_JSON_RUN2And3} GN3XPV01_hbb_bjr FlatMassQCDEff_0p4 
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingJsonToolsTest_Run2And3_GN3XPV01_hbb_bjr_FlatMassQCDEff_0p6 
  SCRIPT BTaggingJsonToolsTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_XBB_LATEST_JSON_RUN2And3} GN3XPV01_hbb_bjr FlatMassQCDEff_0p6 
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingJsonToolsTest_Run2_GN3XPV01_hcc_bjr_FlatMassQCDEff_0p17
  SCRIPT BTaggingJsonToolsTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_XCC_LATEST_JSON_RUN2} GN3XPV01_hcc_bjr FlatMassQCDEff_0p17 
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingJsonToolsTest_Run3_GN3XPV01_hcc_bjr_FlatMassQCDEff_0p17 
  SCRIPT BTaggingJsonToolsTester ${FTAG_INPUT_RUN3_DAOD} ${FTAG_XCC_LATEST_JSON_RUN3} GN3XPV01_hcc_bjr FlatMassQCDEff_0p17 
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingSelectionJsonInstantiation_Xbb_Run2And3
  SCRIPT BTaggingSelectionJsonInstantiationTester ${FTAG_XBB_LATEST_JSON_RUN2And3}
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingSelectionJsonInstantiation_Xcc_Run2
  SCRIPT BTaggingSelectionJsonInstantiationTester ${FTAG_XCC_LATEST_JSON_RUN2}
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)
atlas_add_test(
  ut_BTaggingSelectionJsonInstantiation_Xcc_Run3
  SCRIPT BTaggingSelectionJsonInstantiationTester ${FTAG_XCC_LATEST_JSON_RUN3}
  POST_EXEC_SCRIPT noerror.sh
  LOG_IGNORE_PATTERN "^PathResolver +ERROR Locating dev file"
)



unset(FTAG_INPUT_RUN2_DAOD)
unset(FTAG_INPUT_RUN3_DAOD)
unset(FTAG_LATEST_CDI_RUN2_PCBT)
unset(FTAG_LATEST_CDI_RUN3_PCBT)
unset(FTAG_LATEST_CDI_RUN2_PCFT)
unset(FTAG_LATEST_CDI_RUN3_PCFT)
unset(FTAG_XBB_LATEST_JSON_RUN2And3)
unset(FTAG_XCC_LATEST_JSON_RUN2)
unset(FTAG_XCC_LATEST_JSON_RUN3)
