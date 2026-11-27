# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrackParticleAssociationAlgs )

# Component(s) in the package:
atlas_add_component( TrackParticleAssociationAlgs
                     src/*.cxx
                     src/components/*.cxx
                     LINK_LIBRARIES ActsEventLib ActsGeometryInterfacesLib AthContainers AthLinks AthenaBaseComps CaloGeoHelpers CaloUtilsLib FourMomUtils GaudiKernel PFlowUtilsLib RecoToolInterfaces StoreGateLib TrackToCaloLib TrackVertexAssociationToolLib TrkCaloExtension TrkParameters xAODAssociations xAODCaloEvent xAODTracking )

# Install files from the package:
atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
