# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( CaloTrkMuIdTools )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )
find_package( onnxruntime )

# Component(s) in the package:
atlas_add_component( CaloTrkMuIdTools
                     src/*.cxx
                     src/components/*.cxx
                     INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${ONNXRUNTIME_INCLUDE_DIRS}
                     LINK_LIBRARIES ${ROOT_LIBRARIES} ${ONNXRUNTIME_LIBRARIES} CaloEvent AthenaBaseComps StoreGateLib xAODTracking GaudiKernel ICaloTrkMuIdTools RecoToolInterfaces TrkExInterfaces CaloDetDescrLib CaloGeoHelpers CaloIdentifier CaloUtilsLib xAODCaloEvent ParticleCaloExtension TileDetDescr PathResolver TrkSurfaces TrkCaloExtension TrkParameters AthOnnxInterfaces AsgServicesLib FourMomUtils AthenaKernel CxxUtils)

