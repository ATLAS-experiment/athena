# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

atlas_subdir( PixelNNMonitoring )

atlas_add_component( PixelNNMonitoring
   src/*.cxx src/components/*.cxx
   LINK_LIBRARIES AthenaMonitoringLib AthenaMonitoringKernelLib
                  AthenaBaseComps GaudiKernel StoreGateLib
                  BeamSpotConditionsData EventPrimitives GeoPrimitives
                  InDetPrepRawData InDetReadoutGeometry
                  InDetSimEvent InDetIdentifier ReadoutGeometryBase
                  SiClusterizationToolLib TruthUtils
                  TrkEventPrimitives TrkEventUtils TrkRIO_OnTrack
                  TrkSurfaces TrkTrack )

atlas_install_python_modules( python/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )
