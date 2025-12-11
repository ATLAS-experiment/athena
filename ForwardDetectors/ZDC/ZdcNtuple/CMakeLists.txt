# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

atlas_subdir( ZdcNtuple )

atlas_add_library (ZdcNtupleLib
                   ZdcNtuple/*.h Root/*.cxx
		   PUBLIC_HEADERS ZdcNtuple
		   LINK_LIBRARIES AnaAlgorithmLib AsgAnalysisInterfaces
                   AsgTools PMGAnalysisInterfacesLib PMGToolsLib GeneratorObjects
                   TrigConfInterfaces TrigDecisionToolLib
                   ZdcAnalysisLib xAODCaloEvent xAODEventInfo ZdcConditions ZdcUtilsLib
                   xAODForward xAODHIEvent xAODTracking InDetTrackSelectionToolLib
                   xAODTrigL1Calo xAODTrigMinBias xAODTrigger xAODTruth TrigConfxAODLib
                   PRIVATE_LINK_LIBRARIES xAODCore xAODRootAccess AthContainers)

if (XAOD_STANDALONE)
atlas_add_dictionary (ZdcNtupleDict
			ZdcNtuple/ZdcNtupleDict.h
			ZdcNtuple/selection.xml
			LINK_LIBRARIES ZdcNtupleLib)
endif ()

if (NOT XAOD_STANDALONE)
  # Add a component library for AthAnalysis only:
atlas_add_component (ZdcNtuple
    src/components/*.cxx
    LINK_LIBRARIES ZdcNtupleLib)
endif ()

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
atlas_install_scripts( scripts/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
