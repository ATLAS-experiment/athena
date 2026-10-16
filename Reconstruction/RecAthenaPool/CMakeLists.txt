# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( RecAthenaPool )

# Component(s) in the package:
atlas_add_poolcnv_library( RecAthenaPoolPoolCnv
   src/*.h src/*.cxx
   FILES muonEvent/MuonSpShowerContainer.h muonEvent/MuonCaloEnergyContainer.h
         muonEvent/MuonContainer.h src/MuonCaloEnergies.h
   TYPES_WITH_NAMESPACE Rec::MuonSpShowerContainer Analysis::MuonContainer
                        TPCnv::MuonCaloEnergies
   LINK_LIBRARIES AthenaPoolUtilities StoreGateLib AthenaPoolCnvSvcLib
                  GaudiKernel muonEvent RecTPCnv )

# Install files from the package:
atlas_install_scripts( test/*.py POST_BUILD_CMD ${ATLAS_FLAKE8} )

