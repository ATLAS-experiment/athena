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

  template<ColumnarMode CM> using MuonRange = ObjectRange<MuonDef,CM>;
  template<ColumnarMode CM> using MuonId = ObjectId<MuonDef,CM>;
  template<ColumnarMode CM> using OptMuonId = OptObjectId<MuonDef,CM>;
  template<typename CT,ColumnarMode CM> using MuonAccessor  = AccessorTemplate<MuonDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using MuonDecorator = AccessorTemplate<MuonDef,CT,ColumnAccessMode::output,CM>;
}

#endif
