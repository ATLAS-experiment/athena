# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name.
atlas_subdir( TrkVertexFitters )

#Component(s) in the package.
atlas_add_library( TrkVertexFittersLib
   TrkVertexFitters/*.h src/*.h src/*.cxx
   PUBLIC_HEADERS TrkVertexFitters
   LINK_LIBRARIES AthenaBaseComps xAODTracking GaudiKernel TrkParameters
   TrkParametersBase TrkParticleBase TrkVertexFitterInterfaces
   PRIVATE_LINK_LIBRARIES VxVertex TrkSurfaces TrkLinks TrkTrack VxMultiVertex
   TrkExInterfaces TestTools CxxUtils)

atlas_add_component( TrkVertexFitters
   src/components/*.cxx
   LINK_LIBRARIES TrkVertexFittersLib )

# Test(s) in the package.
atlas_add_test( AdaptiveVertexFitter_test
   SCRIPT python -m TrkConfig.TrkVertexFittersConfig --test AVF
   PROPERTIES TIMEOUT 600
   POST_EXEC_SCRIPT noerror.sh )

atlas_add_test( AdaptiveMultiVertexFitter_test
   SCRIPT python -m TrkConfig.TrkVertexFittersConfig --test AMVF
   PROPERTIES TIMEOUT 300
   POST_EXEC_SCRIPT noerror.sh )
