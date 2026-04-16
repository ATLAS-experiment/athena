# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ParticleJetTools )

find_package( ROOT )

# Component(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_library( ParticleJetToolsLib
      ParticleJetTools/*.h Root/*.cxx
      PUBLIC_HEADERS ParticleJetTools
      LINK_LIBRARIES AsgTools EventPrimitives xAODJet xAODTracking xAODTruth
      JetInterface MCTruthClassifierLib JetRecLib AsgDataHandlesLib
       
      PRIVATE_LINK_LIBRARIES AthContainers CxxUtils TruthUtils 
      )
elseif( GENERATIONBASE )
   # AthGeneration excludes the InDetTrackSystematicsTools package, so the
   # truth vertex decorator algs (TruthVertexDecoratorAlg uses
   # InDet::InDetTrackTruthOriginTool, JetTruthVertexSummaryDecoratorAlg is
   # paired with it) are skipped here. They are derivation-only — AthGeneration
   # does not run derivations.
   file( GLOB _src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} src/*.cxx )
   list( REMOVE_ITEM _src_files src/TruthVertexDecoratorAlg.cxx src/JetTruthVertexSummaryDecoratorAlg.cxx )
   atlas_add_library( ParticleJetToolsLib
      ParticleJetTools/*.h Root/*.cxx ${_src_files}
      PUBLIC_HEADERS ParticleJetTools
      LINK_LIBRARIES AsgDataHandlesLib AsgTools AthContainers AthenaBaseComps EventPrimitives
      GaudiKernel JetInterface JetRecLib MCTruthClassifierLib
      xAODBase xAODEventInfo xAODJet xAODTruth
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AthAnalysisBaseCompsLib CxxUtils TruthUtils xAODTracking )
else()
   atlas_add_library( ParticleJetToolsLib
      ParticleJetTools/*.h Root/*.cxx src/*.cxx
      PUBLIC_HEADERS ParticleJetTools
      LINK_LIBRARIES AsgDataHandlesLib AsgTools AthContainers AthenaBaseComps EventPrimitives
      GaudiKernel JetInterface JetRecLib MCTruthClassifierLib
      xAODBase xAODEventInfo xAODJet xAODTruth
      TruthClassificationLib InDetTrackSystematicsToolsLib
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AsgMessagingLib AthAnalysisBaseCompsLib CxxUtils TruthUtils xAODTracking )
endif()

if( NOT XAOD_STANDALONE )
  if( GENERATIONBASE )
    atlas_add_component( ParticleJetTools
      src/components/*.cxx
      LINK_LIBRARIES ParticleJetToolsLib )
  else()
    atlas_add_component( ParticleJetTools
      src/TruthVertexDecoratorAlg.cxx
      src/JetTruthVertexSummaryDecoratorAlg.cxx
      src/components/*.cxx
      LINK_LIBRARIES ParticleJetToolsLib )
  endif()
endif()

atlas_add_dictionary( ParticleJetToolsDict
  ParticleJetTools/ParticleJetToolsDict.h
  ParticleJetTools/selection.xml
  LINK_LIBRARIES ParticleJetToolsLib )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
