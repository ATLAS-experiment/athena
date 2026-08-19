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
  struct TrackDef : RegularContainerId<xAOD::TrackParticle,xAOD::TrackParticleContainer>
  {
    static constexpr std::string_view idName = "track0";
  };
  using Track0Def = TrackDef;

  struct Track1Def : TrackDef
  {
    static constexpr std::string_view idName = "track1";
  };

  struct Track2Def : TrackDef
  {
    static constexpr std::string_view idName = "track2";
  };

  struct Track3Def : TrackDef
  {
    static constexpr std::string_view idName = "track3";
  };

  struct VertexDef : RegularContainerId<xAOD::Vertex,xAOD::VertexContainer>
  {
    static constexpr std::string_view idName = "vertex";
  };

  using TrackId = ObjectId<TrackDef, ColumnarModeDefault>;
  using OptTrackId = OptObjectId<TrackDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using TrackAccessor  = AccessorTemplate<TrackDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using TrackDecorator = AccessorTemplate<TrackDef,CT,ColumnAccessMode::output,CM>;

  using OptTrack0Id = OptObjectId<Track0Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track0Accessor  = AccessorTemplate<Track0Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track0Decorator = AccessorTemplate<Track0Def,CT,ColumnAccessMode::output,CM>;

  using OptTrack1Id = OptObjectId<Track1Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track1Accessor  = AccessorTemplate<Track1Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track1Decorator = AccessorTemplate<Track1Def,CT,ColumnAccessMode::output,CM>;

  using OptTrack2Id = OptObjectId<Track2Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track2Accessor  = AccessorTemplate<Track2Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track2Decorator = AccessorTemplate<Track2Def,CT,ColumnAccessMode::output,CM>;

  using OptTrack3Id = OptObjectId<Track3Def>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track3Accessor  = AccessorTemplate<Track3Def,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using Track3Decorator = AccessorTemplate<Track3Def,CT,ColumnAccessMode::output,CM>;

  using VertexId = ObjectId<VertexDef, ColumnarModeDefault>;
  using OptVertexId = OptObjectId<VertexDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using VertexAccessor  = AccessorTemplate<VertexDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using VertexDecorator = AccessorTemplate<VertexDef,CT,ColumnAccessMode::output,CM>;
}

#endif
