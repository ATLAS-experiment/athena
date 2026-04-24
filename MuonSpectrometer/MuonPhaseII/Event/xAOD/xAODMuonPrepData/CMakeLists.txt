# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonPrepData )

# External dependencies:
find_package( xAODUtilities )

atlas_add_library( xAODMuonPrepData
    xAODMuonPrepData/*.h xAODMuonPrepData/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODMuonPrepData
    LINK_LIBRARIES xAODCore GeoPrimitives MuonPrepRawData 
                   xAODMeasurementBase Identifier MuonReadoutGeometryR4)


    atlas_add_xaod_smart_pointer_dicts(
    INPUT xAODMuonPrepData/selection.xml
    OUTPUT _selectionFile
    CONTAINERS "xAOD::MdtDriftCircleContainer_v1" 
               "xAOD::RpcStripContainer_v1" 
               "xAOD::TgcStripContainer_v1" 
               "xAOD::sTgcStripContainer_v1" 
               "xAOD::sTgcWireContainer_v1"
               "xAOD::sTgcPadContainer_v1"                
               "xAOD::MM_ClusterContainer_v1")

atlas_add_dictionary( xAODMuonPrepDataDict
    xAODMuonPrepData/xAODMuonPrepDataDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODMuonPrepData
    EXTRA_FILES Root/dict/*.cxx )

