# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigTopoEventTPCnv )

# Component(s) in the package:
atlas_add_library( TrigTopoEventTPCnv
                   src/*.cxx
                   PUBLIC_HEADERS TrigTopoEventTPCnv
                   PRIVATE_LINK_LIBRARIES AthenaKernel
                   LINK_LIBRARIES AthenaPoolCnvSvcLib TrigTopoEvent )

atlas_add_tpcnv_library( TrigTopoEventTPCnvFactories
                         src/factories/*.cxx
                         PUBLIC_HEADERS TrigTopoEventTPCnv
                         LINK_LIBRARIES TrigTopoEventTPCnv )

atlas_add_dictionary( TrigTopoEventTPCnvDict
                      TrigTopoEventTPCnv/TrigTopoEventTPCnvDict.h
                      TrigTopoEventTPCnv/selection.xml
                      LINK_LIBRARIES TrigTopoEventTPCnv )

# Test(s) in the package:
atlas_add_test( ElectronMuonTopoInfoCnv_p1_test
                SOURCES
                test/ElectronMuonTopoInfoCnv_p1_test.cxx
                LINK_LIBRARIES TrigTopoEventTPCnv TestTools CxxUtils )
