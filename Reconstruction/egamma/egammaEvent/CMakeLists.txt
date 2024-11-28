# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( egammaEvent )

# External dependencies:
find_package( ROOT COMPONENTS Core )

# Component(s) in the package:
atlas_add_library( egammaEvent
                   src/*.cxx
                   PUBLIC_HEADERS egammaEvent
                   INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES ${ROOT_LIBRARIES} CaloEvent CaloGeoHelpers CaloIdentifier AthContainers AthLinks Navigation 
                   AthenaKernel EventKernel FourMom NavFourMom ParticleEvent JetEvent Particle TrkParametersBase TrkParameters VxVertex
                   PRIVATE_LINK_LIBRARIES GaudiKernel TrkSurfaces TrkMaterialOnTrack TrkNeutralParameters TrkParticleBase TrkTrack )

atlas_add_dictionary( egammaEventDict
                      egammaEvent/egammaEventDict.h
                      egammaEvent/selection.xml
                      LINK_LIBRARIES egammaEvent
                      NAVIGABLES egammaContainer ElectronContainer PhotonContainer EMShowerMinimalContainer
                      ELEMENT_LINKS egDetailContainer CaloRingsContainer )

atlas_add_dictionary( egammaEnumsDict
                      egammaEvent/egammaEnumsDict.h
                      egammaEvent/selectionEnums.xml
                      LINK_LIBRARIES egammaEvent )
