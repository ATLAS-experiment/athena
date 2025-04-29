/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_CORE_PARTICLE_DEF_H
#define COLUMNAR_CORE_PARTICLE_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODBase/IParticleContainer.h>

namespace columnar
{
  template<ContainerId CI> requires (CI == ContainerId::particle0 || CI == ContainerId::particle1)
  struct ContainerIdTraits<CI> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::IParticle;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::IParticleContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::IParticleContainer;
 };
}

#endif
