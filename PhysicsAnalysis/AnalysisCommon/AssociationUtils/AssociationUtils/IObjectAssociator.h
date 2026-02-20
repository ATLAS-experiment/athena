/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_IOBJECTASSOCIATOR_H
#define ASSOCIATIONUTILS_IOBJECTASSOCIATOR_H

// EDM includes
#include <xAODBase/ObjectType.h>

// Columnar includes
#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ParticleDef.h"

namespace ORUtils
{

  /// @class IParticleAssociator
  /// @brief Interface for a class which checks for a match between IParticles.
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class IParticleAssociator : public columnar::ColumnarTool<>
  {
    public:
      /// Virtual destructor
      virtual ~IParticleAssociator(){};

      /// Set the object types to be used in the association.
      ///
      /// This must be called before using the associator in columnar mode.
      virtual StatusCode setObjectTypes (xAODType::ObjectType type1,
                                   xAODType::ObjectType type2) = 0;

      /// Test association between two IParticles
      virtual bool objectsMatch(columnar::Particle1Id p1,
                                columnar::Particle2Id p2, bool swapArgs = false) const = 0;
      bool objectsMatch(columnar::Particle2Id p2,
                        columnar::Particle1Id p1) const
      { return objectsMatch(p1, p2, true); }
  };

} // namespace ORUtils

#endif
