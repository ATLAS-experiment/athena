# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODBTaggingEfficiency )

# External dependencies:
find_package( ROOT COMPONENTS Cint Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint )
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


if( NOT XAOD_STANDALONE )
   atlas_add_component( xAODBTaggingEfficiency
      src/*.h src/*.cxx src/components/*.cxx
      LINK_LIBRARIES xAODJet CalibrationDataInterfaceLib AthenaBaseComps
      GaudiKernel PathResolver xAODBTaggingEfficiencyLib FTagAnalysisInterfacesLib )
endif()

atlas_add_dictionary( xAODBTaggingEfficiencyDict
  xAODBTaggingEfficiency/xAODBTaggingEfficiencyDict.h
  xAODBTaggingEfficiency/selection.xml
  LINK_LIBRARIES xAODBTaggingEfficiencyLib )

# Executable(s) in the package (to be built only under AthAnalysis or in stand-alone mode):
if( XAOD_ANALYSIS )
   atlas_add_executable( BTaggingToolsTester
     util/BTaggingToolsTester.cxx 
     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
     LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib )

   atlas_add_executable( BTaggingEfficiencyToolTester
      util/BTaggingEfficiencyToolTester.cxx
      LINK_LIBRARIES xAODRootAccess AsgTools FTagAnalysisInterfacesLib )
   atlas_add_executable( BTaggingJsonToolsTester
      util/BTaggingJsonToolsTester.cxx
      LINK_LIBRARIES xAODRootAccess AsgTools FTagAnalysisInterfacesLib )

   atlas_add_executable( SystematicStrategyComparison
      util/SystematicStrategyComparison.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib )

   atlas_add_executable( BTaggingEigenVectorRecompositionToolTester
      util/BTaggingEigenVectorRecompositionToolTester.cxx
      LINK_LIBRARIES AsgTools FTagAnalysisInterfacesLib )

   atlas_add_executable( BTaggingSelectionToolTester
      util/BTaggingSelectionToolTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib )

   atlas_add_executable(
      BTaggingToolsExample
      util/BTaggingToolsExample.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} AsgTools xAODJet xAODBTagging PATInterfaces FTagAnalysisInterfacesLib
   )

   if( XAOD_STANDALONE )
      target_link_libraries( BTaggingToolsTester 
         PRIVATE xAODRootAccess )
      target_link_libraries( BTaggingSelectionToolTester
         PRIVATE xAODRootAccess )
      target_link_libraries( BTaggingToolsExample
         PRIVATE xAODRootAccess )
      target_link_libraries( BTaggingJsonToolsTester
         PRIVATE xAODRootAccess )
      target_link_libraries( SystematicStrategyComparison 
         PRIVATE xAODRootAccess )
   else()
      target_link_libraries( BTaggingToolsTester 
         PRIVATE POOLRootAccessLib )
      target_link_libraries( BTaggingSelectionToolTester
         PRIVATE POOLRootAccessLib )
      target_link_libraries( BTaggingToolsExample
         PRIVATE POOLRootAccessLib )
       target_link_libraries( BTaggingJsonToolsTester
         PRIVATE POOLRootAccessLib )
      target_link_libraries( SystematicStrategyComparison 
        PRIVATE POOLRootAccessLib )

      atlas_add_executable( BTaggingTruthTaggingTester
        util/BTaggingTruthTaggingTester.cxx
        LINK_LIBRARIES AsgTools xAODBTaggingEfficiencyLib POOLRootAccessLib AthAnalysisBaseCompsLib xAODEventInfo xAODRootAccess AsgMessagingLib FTagAnalysisInterfacesLib )
   endif()

   # Test for both AthAnalysis and AnalysisBase 
   atlas_add_test( ut_BTaggingToolsExample
      SCRIPT BTaggingToolsExample /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p6491/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p6491/DAOD_PHYS.41633384._000880.pool.root.1
      # No post execution script 
      POST_EXEC_SCRIPT nopost.sh
   )
endif()

if( XAOD_STANDALONE )
  atlas_add_test( ut_BTaggingJsonToolsTest
    SCRIPT BTaggingJsonToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p6491/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p6491/DAOD_PHYS.41633384._000880.pool.root.1 /cvmfs/atlas.cern.ch/repo/sw/database/GroupData/xAODBTaggingEfficiency/13And13p6TeV/MC20And23_2025-03-17_GN2Xv01-noSF_v1.json GN2Xv01 FlatMassQCDEff_0p25
  )
  atlas_add_test( ut_BTaggingToolsTest 
    SCRIPT BTaggingToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p6491/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p6491/DAOD_PHYS.41633384._000880.pool.root.1 xAODBTaggingEfficiency/13p6TeV/MC23_2025-06-17_GN2v01_v4.root GN2v01 FixedCutBEff_77 1d
  )
   atlas_add_test( ut_BTaggingSelectionTool_continuous
     SCRIPT BTaggingSelectionToolTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/ASG/DAOD_PHYS/p6491/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.deriv.DAOD_PHYS.e8514_e8528_s4162_s4114_r15540_r15516_p6491/DAOD_PHYS.41633384._000880.pool.root.1 xAODBTaggingEfficiency/13p6TeV/MC23_2024-10-17_GN2v01_v1.root GN2v01 Continuous
  )
  atlas_add_test( ut_BTaggingEfficiencyTool
     SCRIPT BTaggingEfficiencyToolTester xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root GN2v00NewAliasWP FixedCutBEff_77
   )
endif()


# Install files from the package:
atlas_install_runtime( share/*.root )
