/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_IOVERLAPTOOL_H
#define ASSOCIATIONUTILS_IOVERLAPTOOL_H

// Framework includes
#include "AsgTools/IAsgTool.h"
#include "AsgTools/CurrentContext.h"

// EDM includes
#include "xAODBase/IParticleContainer.h"

// Columnar includes
#include "ColumnarCore/ObjectRange.h"
#include "ColumnarCore/ParticleDef.h"

namespace ORUtils
{

  /// @class IOverlapTool
  /// @brief Interface class for overlap removal tools.
  ///
  /// Tools that implement this interface will operate on particle containers
  /// and find and mark overlaps based on their custom logic.
  ///
  /// @author Steve Farrell <Steven.Farrell@cern.ch>
  ///
  class IOverlapTool : virtual public asg::IAsgTool
  {

      /// Declare the interface
      ASG_TOOL_INTERFACE(IOverlapTool)

    public:

      /// @brief Find overlaps between two containers.
      /// The details will depend on the tool implementation, but the convention
      /// should be that if only one container is to be marked, it should be the
      /// first argument. Otherwise, both of them can be marked. The decoration
      /// logic may depend on configuration.
      virtual StatusCode findOverlaps(columnar::Particle1Range cont1,
                                      columnar::Particle2Range cont2,
                                      columnar::EventContextId eventContext) const = 0;
      StatusCode findOverlaps(const xAOD::IParticleContainer& cont1,
                              const xAOD::IParticleContainer& cont2,
                              const EventContext& eventContext = Gaudi::Hive::currentContext()) const {
        return findOverlaps(
          columnar::Particle1Range(cont1),
          columnar::Particle2Range(cont2),
          eventContext);
      }

  }; // class IOverlapTool

} // namespace ORUtils

#endif
