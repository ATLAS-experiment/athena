#Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( MuonDQAUtils )

# External dependencies:
find_package( ROOT COMPONENTS Core Hist Physics )

# Component(s) in the package:
atlas_add_library( MuonDQAUtilsLib
                   src/*.cxx
                   PUBLIC_HEADERS MuonDQAUtils
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} AnalysisTriggerEvent AthenaBaseComps EventKernel GaudiKernel JetEvent JetTagEvent Particle ParticleEvent TrigMuonEvent egammaEvent muonEvent )

atlas_add_component( MuonDQAUtils
                     src/components/*.cxx
                     LINK_LIBRARIES MuonDQAUtilsLib )
