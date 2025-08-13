/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <exception>

// Athena/Gaudi include(s):
#include "AthenaKernel/IThinningSvc.h"
#include "AthContainers/tools/copyThinned.h"

// EDM include(s).
#include "xAODTruth/versions/TruthVertexAuxContainer_v1.h"
#include "xAODTruth/versions/TruthVertexAuxContainer_v2.h"

// Local include(s):
#include "xAODTruthVertexAuxContainerCnv.h"
#include "xAODTruthVertexAuxContainerCnv_v2.h"

xAODTruthVertexAuxContainerCnv::
xAODTruthVertexAuxContainerCnv( ISvcLocator* svcLoc )
   : xAODTruthVertexAuxContainerCnvBase( svcLoc ) {

}

xAOD::TruthVertexAuxContainer*
xAODTruthVertexAuxContainerCnv::
createPersistent( xAOD::TruthVertexAuxContainer* trans ) {

   // Access the thinning svc, if thinning is defined for this object:
   IThinningSvc* thinSvc = IThinningSvc::instance();

   // Create the persistent object using the helper function from AthContainers:
   return SG::copyThinned( *trans, thinSvc );
}

xAOD::TruthVertexAuxContainer*
xAODTruthVertexAuxContainerCnv::createTransient() {

   // The known ID(s) for this container:
   static const pool::Guid v1_guid( "B6BD3B02-C411-4EB9-903F-5B099D3B1A3E" );
   static const pool::Guid v2_guid( "95326A79-5433-4F65-9590-8E72D67FE0D5" );

   // Check which version of the container we're reading:
   if( compareClassGuid( v1_guid ) ) {
      // It's the latest version, read it directly:
      return poolReadObject< xAOD::TruthVertexAuxContainer_v1 >();
   } else if( compareClassGuid( v2_guid ) ) {
      // Set up the v2 converter.
      static xAODTruthVertexAuxContainerCnv_v2 converter;
      // Convert the v2 version "back" to the v1 version.
      std::unique_ptr<xAOD::TruthVertexAuxContainer_v2>
         pers{poolReadObject<xAOD::TruthVertexAuxContainer_v2>()};
      return converter.createTransient(pers.get(), msg());
   }

   // If we didn't recognise the ID:
   throw std::runtime_error( "Unsupported version of "
                             "xAOD::TruthVertexAuxContainer found" );
   return 0;
}
