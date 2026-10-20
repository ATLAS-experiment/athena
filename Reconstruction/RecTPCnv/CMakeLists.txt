# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RecTPCnv )

# Component(s) in the package:
atlas_add_library( RecTPCnv
                   src/*.cxx
                   PUBLIC_HEADERS RecTPCnv
                   LINK_LIBRARIES AthenaPoolCnvSvcLib TrkEventTPCnv TrkMaterialOnTrack muonEvent
                   PRIVATE_LINK_LIBRARIES AthenaKernel RootConversions )

atlas_add_tpcnv_library( RecTPCnvFactories
                         src/factories/*.cxx
                         PUBLIC_HEADERS RecTPCnv
                         LINK_LIBRARIES RecTPCnv )

atlas_add_dictionary( RecTPCnvDict
                      RecTPCnv/RecTPCnvDict.h
                      RecTPCnv/selection.xml
                      LINK_LIBRARIES RecTPCnv )

atlas_add_dictionary( OLD_RecTPCnvDict
                      RecTPCnv/RecTPCnvDict.h
                      RecTPCnv/OLD_selection.xml
                      LINK_LIBRARIES RecTPCnv )

# Tests in the package:
function( _add_test test )
   atlas_add_test( ${test}
                   SOURCES
                   test/${test}.cxx
                   LINK_LIBRARIES RecTPCnv TestTools
                   LOG_IGNORE_PATTERN "of type MuonCaloEnergyContainer" )
endfunction()

_add_test( CaloEnergyCnv_p1_test )
_add_test( CaloEnergyCnv_p2_test )
_add_test( DepositInCaloCnv_p1_test )
_add_test( DepositInCaloCnv_p2_test )
_add_test( MuonCaloEnergyContainerCnv_p1_test )
