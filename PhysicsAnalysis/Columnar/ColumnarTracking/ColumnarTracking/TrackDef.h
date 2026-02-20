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
  namespace ContainerId
  {
    struct track : regularCIBase<xAOD::TrackParticle,xAOD::TrackParticleContainer>
    {
      static constexpr std::string_view idName = "track0";
    };
    using track0 = track;

    struct track1 : track
    {
      static constexpr std::string_view idName = "track1";
    };

    struct track2 : track
    {
      static constexpr std::string_view idName = "track2";
    };

    struct track3 : track
    {
      static constexpr std::string_view idName = "track3";
    };

    struct vertex : regularCIBase<xAOD::Vertex,xAOD::VertexContainer>
    {
      static constexpr std::string_view idName = "vertex";
    };
  }

  using TrackId = ObjectId<ContainerId::track>;
  using OptTrackId = OptObjectId<ContainerId::track>;
  template<typename CT,typename CM=ColumnarModeDefault> using TrackAccessor  = AccessorTemplate<ContainerId::track,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using TrackDecorator = AccessorTemplate<ContainerId::track,CT,ColumnAccessMode::output,CM>;

  using OptTrack0Id = OptObjectId<ContainerId::track0>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track0Accessor  = AccessorTemplate<ContainerId::track0,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track0Decorator = AccessorTemplate<ContainerId::track0,CT,ColumnAccessMode::output,CM>;

  using OptTrack1Id = OptObjectId<ContainerId::track1>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track1Accessor  = AccessorTemplate<ContainerId::track1,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track1Decorator = AccessorTemplate<ContainerId::track1,CT,ColumnAccessMode::output,CM>;

  using OptTrack2Id = OptObjectId<ContainerId::track2>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track2Accessor  = AccessorTemplate<ContainerId::track2,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track2Decorator = AccessorTemplate<ContainerId::track2,CT,ColumnAccessMode::output,CM>;

  using OptTrack3Id = OptObjectId<ContainerId::track3>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track3Accessor  = AccessorTemplate<ContainerId::track3,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track3Decorator = AccessorTemplate<ContainerId::track3,CT,ColumnAccessMode::output,CM>;

  using VertexId = ObjectId<ContainerId::vertex>;
  using OptVertexId = OptObjectId<ContainerId::vertex>;
  template<typename CT,typename CM=ColumnarModeDefault> using VertexAccessor  = AccessorTemplate<ContainerId::vertex,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using VertexDecorator = AccessorTemplate<ContainerId::vertex,CT,ColumnAccessMode::output,CM>;
}

#endif
