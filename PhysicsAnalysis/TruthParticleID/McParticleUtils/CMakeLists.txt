# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( McParticleUtils )

# Component(s) in the package:
atlas_add_library( McParticleUtils
                   src/*.cxx
                   PUBLIC_HEADERS McParticleUtils
                   LINK_LIBRARIES AnalysisUtilsLib AthContainers AthenaBaseComps AtlasHepMCLib EventKernel
                   PRIVATE_LINK_LIBRARIES AthenaKernel GaudiKernel RootUtils )
