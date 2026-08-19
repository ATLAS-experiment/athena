/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_MUON_MUON_DEF_H
#define COLUMNAR_MUON_MUON_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODMuon/MuonContainer.h>

namespace columnar
{
  struct MuonDef : RegularContainerId<xAOD::Muon,xAOD::MuonContainer>
  {
    static constexpr std::string_view idName = "muon";
  };

  using MuonRange = ObjectRange<MuonDef, ColumnarModeDefault>;
  using MuonId = ObjectId<MuonDef, ColumnarModeDefault>;
  using OptMuonId = OptObjectId<MuonDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using MuonAccessor  = AccessorTemplate<MuonDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MuonDecorator = AccessorTemplate<MuonDef,CT,ColumnAccessMode::output,CM>;
}

#endif
