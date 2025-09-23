# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODTruthCnv )

# Component(s) in the package:
if( SIMULATIONBASE )
   atlas_add_library( xAODTruthCnvLib
      xAODTruthCnv/TruthMetaDataTool.h Root/TruthMetaDataTool.cxx
      PUBLIC_HEADERS xAODTruthCnv
      LINK_LIBRARIES AsgTools xAODTruth TruthUtils )
else()
   atlas_add_library( xAODTruthCnvLib
      xAODTruthCnv/*.h Root/*.cxx
      PUBLIC_HEADERS xAODTruthCnv
      LINK_LIBRARIES AsgTools xAODTruth TruthUtils AnaAlgorithmLib
         AsgDataHandlesLib )
endif()

if( XAOD_STANDALONE )
   atlas_add_component( xAODTruthCnv
      src/components/xAODTruthCnvUniversal_entries.cxx
      src/components/xAODTruthCnvAnalysis_entries.cxx
      LINK_LIBRARIES xAODTruthCnvLib )
elseif( SIMULATIONBASE )
   atlas_add_component( xAODTruthCnv
      src/*.h src/*.cxx
      src/components/xAODTruthCnvUniversal_entries.cxx
      src/components/xAODTruthCnvAthena_entries.cxx
      LINK_LIBRARIES AthLinks AthenaBaseComps StoreGateLib EventInfo
         xAODEventInfo xAODTruth GaudiKernel GeneratorObjects
         AthenaPoolUtilities IOVDbDataModel xAODTruthCnvLib TruthUtils )
else()
   atlas_add_component( xAODTruthCnv
      src/*.h src/*.cxx
      src/components/*.cxx
      LINK_LIBRARIES AthLinks AthenaBaseComps StoreGateLib EventInfo
         xAODEventInfo xAODTruth GaudiKernel GeneratorObjects
         AthenaPoolUtilities IOVDbDataModel xAODTruthCnvLib TruthUtils )
endif()

# Install files from the package:
atlas_install_scripts( scripts/dumpTruth.py  )
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
