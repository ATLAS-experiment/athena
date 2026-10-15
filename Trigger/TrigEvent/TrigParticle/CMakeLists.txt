# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( TrigParticle )

# Component(s) in the package:
atlas_add_library( TrigParticle
                   src/*.cxx
                   PUBLIC_HEADERS TrigParticle
                   LINK_LIBRARIES AthContainers AthLinks AthenaKernel EventKernel FourMom Navigation TrigCaloEvent TrigInDetEvent egammaEvent
                   PRIVATE_LINK_LIBRARIES CaloGeoHelpers JetEvent TrigNavigationLib TruthUtils )

atlas_generate_cliddb( TrigParticle )

atlas_add_sercnv_library( TrigParticleSerCnv
                          FILES TrigParticle/TrigElectron.h TrigParticle/TrigElectronContainer.h TrigParticle/TrigPhoton.h TrigParticle/TrigPhotonContainer.h egammaEvent/egammaContainer.h egammaEvent/egDetailContainer.h JetEvent/JetCollection.h JetEvent/JetMomentMap.h JetEvent/JetMomentMapCollection.h JetEvent/JetKeyDescriptor.h JetEvent/JetKeyDescriptorCollection.h
                          LINK_LIBRARIES TrigParticle )

atlas_add_dictionary( TrigParticleDict
                      TrigParticle/TrigParticleDict.h
                      TrigParticle/selection.xml
                      LINK_LIBRARIES TrigParticle
                      DATA_LINKS TrigPhotonContainer
                      ELEMENT_LINKS TrigElectronContainer )

