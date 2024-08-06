# Copyright (C) 2002-, 20242024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RecEventTPCnv )

# Component(s) in the package:
atlas_add_library( RecEventTPCnv
                   src/*.cxx
                   PUBLIC_HEADERS RecEventTPCnv
                   LINK_LIBRARIES AthenaPoolCnvSvcLib RecEvent AthenaKernel )

atlas_add_tpcnv_library( RecEventTPCnvFactories
                         src/factories/*.cxx
                         PUBLIC_HEADERS RecEventTPCnv
                         LINK_LIBRARIES RecEventTPCnv )

atlas_add_dictionary( RecEventTPCnvDict
                      RecEventTPCnv/RecEventTPCnvDict.h
                      RecEventTPCnv/selection.xml
                      LINK_LIBRARIES RecEvent RecEventTPCnv )
