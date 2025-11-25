# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonRDO )

# External dependencies:
find_package( xAODUtilities )

atlas_add_library( xAODMuonRDO
    xAODMuonRDO/*.h xAODMuonRDO/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODMuonRDO
    LINK_LIBRARIES xAODCore GeoPrimitives xAODMeasurementBase Identifier MuonNSWCommonDecode)

  
atlas_add_xaod_smart_pointer_dicts(
    INPUT xAODMuonRDO/selection.xml
    OUTPUT _selectionFile
    CONTAINERS "xAOD::NRPCRDOContainer_v1" )

atlas_add_dictionary( xAODMuonRDODict
    xAODMuonRDO/xAODMuonRDODict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODMuonRDO
    EXTRA_FILES Root/dict/*.cxx )

# Generate CLIDs for the library explicitly.
atlas_generate_cliddb( xAODMuonRDO )
