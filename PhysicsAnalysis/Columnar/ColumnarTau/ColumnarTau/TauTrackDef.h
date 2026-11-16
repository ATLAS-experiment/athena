/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COLUMNAR_TAU_TAU_TRACK_DEF_H
#define COLUMNAR_TAU_TAU_TRACK_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODTau/TauTrackContainer.h>

namespace columnar
{
  struct TauTrackDef : RegularContainerId<xAOD::TauTrack,xAOD::TauTrackContainer>
  {
    static constexpr std::string_view idName = "tauTrack";
  };
  using TauTrackRange = ObjectRange<TauTrackDef>;
  using TauTrackId = ObjectId<TauTrackDef>;
  using OptTauTrackId = OptObjectId<TauTrackDef>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TauTrackAccessor  = AccessorTemplate<TauTrackDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarModeConcept CM=ColumnarMode> using TauTrackDecorator = AccessorTemplate<TauTrackDef,CT,ColumnAccessMode::output,CM>;
}

#endif
