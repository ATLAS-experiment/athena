# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS Collaboration

# Declare the package name:
atlas_subdir( MuSAVtxFitter )

atlas_add_library( MuSAVtxFitterLib
                   src/*.cxx
                   PUBLIC_HEADERS MuSAVtxFitter 
                   LINK_LIBRARIES AthenaBaseComps GaudiKernel StoreGateLib xAODTracking TrkVertexAnalysisUtilsLib BeamSpotConditionsData TruthUtils InDetConversionFinderToolsLib ITrackToVertex TrkExInterfaces TrkToolInterfaces TrkVertexFitterInterfaces xAODEventInfo xAODMuon TrkVKalVrtFitterLib VrtSecInclusiveLib JpsiUpsilonToolsLib)

atlas_add_component( MuSAVtxFitter
                     src/components/*.cxx
                     LINK_LIBRARIES MuSAVtxFitterLib )

atlas_install_python_modules( python/*.py )

atlas_add_test( MuSAVtxFitter_test
        SCRIPT python -m MuSAVtxFitter.MuSAVtxFitterConfig
        POST_EXEC_SCRIPT noerror.sh)
