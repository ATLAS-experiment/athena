/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TRACKING_TRACK_DEF_H
#define COLUMNAR_TRACKING_TRACK_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

namespace columnar
{
  template<ContainerId CI> requires (CI == ContainerId::track0 || CI == ContainerId::track1 || CI == ContainerId::track2)
  struct ContainerIdTraits<CI> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::TrackParticle;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::TrackParticleContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::TrackParticleContainer;
  };

  template<> struct ContainerIdTraits<ContainerId::vertex> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Vertex;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::VertexContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::VertexContainer;
  };
}

#endif
