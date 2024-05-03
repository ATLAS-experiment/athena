# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODBTaggingEfficiency )

# External dependencies:
find_package( ROOT COMPONENTS Cint Core Tree MathCore Hist RIO pthread MathMore Minuit Minuit2 Matrix Physics HistPainter Rint )
find_package( onnxruntime )

atlas_add_root_dictionary( xAODBTaggingEfficiencyLib xAODBTaggingEfficiencyLibDictSource ROOT_HEADERS xAODBTaggingEfficiency/TruthTagResults.h Root/LinkDef.h )

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
   atlas_add_executable( BTaggingEfficiencyToolTester
      util/BTaggingEfficiencyToolTester.cxx
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

   if( XAOD_STANDALONE )
      target_link_libraries( BTaggingSelectionToolTester
         PRIVATE xAODRootAccess )
      target_link_libraries( SystematicStrategyComparison 
         PRIVATE xAODRootAccess )
   else()
      target_link_libraries( BTaggingSelectionToolTester
         PRIVATE POOLRootAccessLib )
      target_link_libraries( SystematicStrategyComparison 
         PRIVATE POOLRootAccessLib )
   endif()

    atlas_add_executable( BTaggingTruthTaggingTester
      util/BTaggingTruthTaggingTester.cxx
      LINK_LIBRARIES AsgTools AsgMessagingLib FTagAnalysisInterfacesLib )
endif()

if( XAOD_STANDALONE )
   atlas_add_test( ut_BTaggingSelectionTool_fixedcut
      SCRIPT BTaggingSelectionToolTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/main/mc_PHYS_Run3/v14/DAOD_PHYS.myOutput.pool.root xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root GN2v00NewAliasWP FixedCutBEff_77 
   )
   atlas_add_test( ut_BTaggingSelectionTool_continuous
      SCRIPT BTaggingSelectionToolTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/main/mc_PHYS_Run3/v14/DAOD_PHYS.myOutput.pool.root xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root GN2v00NewAliasWP Continuous
   )
   atlas_add_test( ut_BTaggingEfficiencyTool
      SCRIPT BTaggingEfficiencyToolTester xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root GN2v00NewAliasWP FixedCutBEff_77
   )
endif()

# Install files from the package:
atlas_install_runtime( share/*.root )
