# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( HGTD_EventAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( HGTD_EventAthenaPoolPoolCnv
   src/*.h src/*.cxx
   FILES HGTD_RawData/HGTD_RDO_Container.h
         HGTD_RawData/HGTD_ALTIROC_RDO_Container.h
         HGTD_PrepRawData/HGTD_ClusterContainer.h
   LINK_LIBRARIES GaudiKernel AthenaPoolCnvSvcLib HGTD_RawData HGTD_PrepRawData
   HGTD_EventTPCnv )
