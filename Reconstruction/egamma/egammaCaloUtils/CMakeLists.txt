# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( egammaCaloUtils )

atlas_add_library( egammaCaloUtils
   egammaUtils/*.h src/*.cxx
   PUBLIC_HEADERS egammaCaloUtils
   LINK_LIBRARIES CaloDetDescrLib CaloEvent CaloGeoHelpers 
   CaloIdentifier CaloUtilsLib xAODCaloEvent FourMomUtils)

