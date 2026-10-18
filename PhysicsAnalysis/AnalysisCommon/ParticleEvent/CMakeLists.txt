# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( ParticleEvent )


# Component(s) in the package:
atlas_add_library( ParticleEvent
                   src/*.cxx
                   PUBLIC_HEADERS ParticleEvent
		           LINK_LIBRARIES AthContainers AthLinks Navigation AthenaKernel EventKernel FourMom NavFourMom Particle VxVertex CxxUtils )

atlas_add_dictionary( ParticleEventDict
                      ParticleEvent/ParticleEventDict.h
                      ParticleEvent/selection.xml
                      LINK_LIBRARIES ParticleEvent
                      NAVIGABLES DataVector<ParticleBase> NeutrinoContainer 
                      DATA_LINKS TrackParticleAssocs INav4MomToTrackParticleAssocs )
