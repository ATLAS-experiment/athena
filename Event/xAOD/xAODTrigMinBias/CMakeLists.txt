# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( xAODTrigMinBias )

# Pull in the helper CMake code.
find_package( xAODUtilities )

# Component(s) in the package.
atlas_add_library( xAODTrigMinBias
   xAODTrigMinBias/*.h xAODTrigMinBias/versions/*.h Root/*.cxx
   PUBLIC_HEADERS xAODTrigMinBias
   LINK_LIBRARIES AthContainers xAODCore )

atlas_add_xaod_smart_pointer_dicts(
   INPUT xAODTrigMinBias/selection.xml
   OUTPUT _selectionFile
   CONTAINERS "xAOD::TrigSpacePointCountsContainer_v1"
              "xAOD::TrigT2MbtsBitsContainer_v1" "xAOD::TrigHisto2DContainer_v1"
              "xAOD::TrigTrackCountsContainer_v1"
              "xAOD::TrigVertexCountsContainer_v1"
              "xAOD::TrigT2ZdcSignalsContainer_v1" )

atlas_add_dictionary( xAODTrigMinBiasDict
   xAODTrigMinBias/xAODTrigMinBiasDict.h
   ${_selectionFile}
   LINK_LIBRARIES xAODCore xAODTrigMinBias
   EXTRA_FILES Root/dict/*.cxx )

if( NOT XAOD_ANALYSIS )
   atlas_add_sercnv_library( xAODTrigMinBiasSerCnv
      FILES xAODTrigMinBias/TrigSpacePointCounts.h xAODTrigMinBias/TrigSpacePointCountsContainer.h
            xAODTrigMinBias/TrigSpacePointCountsAuxContainer.h
            xAODTrigMinBias/TrigHisto2D.h xAODTrigMinBias/TrigHisto2DContainer.h
            xAODTrigMinBias/TrigHisto2DAuxContainer.h
            xAODTrigMinBias/TrigVertexCounts.h xAODTrigMinBias/TrigVertexCountsContainer.h
            xAODTrigMinBias/TrigVertexCountsAuxContainer.h
            xAODTrigMinBias/TrigTrackCounts.h xAODTrigMinBias/TrigTrackCountsContainer.h
            xAODTrigMinBias/TrigTrackCountsAuxContainer.h
      TYPES_WITH_NAMESPACE
            xAOD::TrigSpacePointCounts xAOD::TrigSpacePointCountsContainer xAOD::TrigSpacePointCountsAuxContainer
            xAOD::TrigHisto2D xAOD::TrigHisto2DContainer xAOD::TrigHisto2DAuxContainer
            xAOD::TrigVertexCounts xAOD::TrigVertexCountsContainer xAOD::TrigVertexCountsAuxContainer
            xAOD::TrigTrackCounts xAOD::TrigTrackCountsContainer xAOD::TrigTrackCountsAuxContainer
      CNV_PFX xAOD
      LINK_LIBRARIES xAODTrigMinBias TrigSerializeCnvSvcLib )
endif()

# Generate CLIDs from the library.
atlas_generate_cliddb( xAODTrigMinBias )
