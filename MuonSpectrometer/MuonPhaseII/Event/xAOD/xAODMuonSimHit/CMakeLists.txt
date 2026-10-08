# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( xAODMuonSimHit )

# External dependencies:
find_package( xAODUtilities )

atlas_add_library( xAODMuonSimHit
    xAODMuonSimHit/*.h xAODMuonSimHit/versions/*.h Root/*.cxx
    PUBLIC_HEADERS xAODMuonSimHit
    LINK_LIBRARIES xAODCore GeoPrimitives GeneratorObjects
                   xAODMeasurementBase Identifier MuonReadoutGeometryR4)

  
atlas_add_xaod_smart_pointer_dicts(
    INPUT xAODMuonSimHit/selection.xml
    OUTPUT _selectionFile
    CONTAINERS "xAOD::MuonSimHitContainer_v1" )

atlas_add_dictionary( xAODMuonSimHitDict
    xAODMuonSimHit/xAODMuonSimHitDict.h
    ${_selectionFile}
    LINK_LIBRARIES xAODCore xAODMuonSimHit
    EXTRA_FILES Root/dict/*.cxx )


find_package( GTest )



atlas_add_test( SimHitTest
                SOURCES test/test_simHit.cxx
                INCLUDE_DIRS ${GTEST_INCLUDE_DIRS}
                LINK_LIBRARIES ${GTEST_LIBRARIES} xAODMuonSimHit AthenaKernel TestTools
                POST_EXEC_SCRIPT noerror.sh)
    

