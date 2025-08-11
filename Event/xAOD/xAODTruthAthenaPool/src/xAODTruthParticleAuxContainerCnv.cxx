/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <exception>

// Athena/Gaudi include(s):
#include "AthenaKernel/IThinningSvc.h"
#include "AthContainers/tools/copyThinned.h"

// EDM include(s).
#include "xAODTruth/versions/TruthParticleAuxContainer_v1.h"
#include "xAODTruth/versions/TruthParticleAuxContainer_v2.h"

// Local include(s):
#include "xAODTruthParticleAuxContainerCnv.h"
#include "xAODTruthParticleAuxContainerCnv_v2.h"

xAODTruthParticleAuxContainerCnv::
xAODTruthParticleAuxContainerCnv( ISvcLocator* svcLoc )
   : xAODTruthParticleAuxContainerCnvBase( svcLoc ) {

}

xAOD::TruthParticleAuxContainer*
xAODTruthParticleAuxContainerCnv::
createPersistent( xAOD::TruthParticleAuxContainer* trans ) {

   // Access the thinning svc, if thinning is defined for this object:
   IThinningSvc* thinSvc = IThinningSvc::instance();

   // Create the persistent object using the helper function from AthContainers:
   return SG::copyThinned( *trans, thinSvc );
}

xAOD::TruthParticleAuxContainer*
xAODTruthParticleAuxContainerCnv::createTransient() {

   // The known ID(s) for this container:
   static const pool::Guid v1_guid( "BA8FA08F-8DD6-420D-97D5-8B54EABECD65" );
   static const pool::Guid v2_guid( "3D5B5EBB-D923-4A12-987C-80791C63F3B6" );

   // Check which version of the container we're reading:
   if( compareClassGuid( v1_guid ) ) {
      // It's the latest version, read it directly:
      return poolReadObject< xAOD::TruthParticleAuxContainer_v1 >();
   } else if( compareClassGuid( v2_guid ) ) {
      // Set up the v2 converter.
      static xAODTruthParticleAuxContainerCnv_v2 converter;
      // Convert the v2 version "back" to the v1 version.
      std::unique_ptr<xAOD::TruthParticleAuxContainer_v2>
         pers{poolReadObject<xAOD::TruthParticleAuxContainer_v2>()};
      return converter.createTransient(pers.get(), msg());
   }

   // If we didn't recognise the ID:
   throw std::runtime_error( "Unsupported version of "
                             "xAOD::TruthParticleAuxContainer found" );
   return 0;
}
