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
  namespace ContainerId
  {
    struct muon : regularCIBase<xAOD::Muon,xAOD::MuonContainer>
    {
      static constexpr std::string_view idName = "muon";
    };
  }

  using MuonRange = ObjectRange<ContainerId::muon>;
  using MuonId = ObjectId<ContainerId::muon>;
  using OptMuonId = OptObjectId<ContainerId::muon>;
  template<typename CT,typename CM=ColumnarModeDefault> using MuonAccessor  = AccessorTemplate<ContainerId::muon,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MuonDecorator = AccessorTemplate<ContainerId::muon,CT,ColumnAccessMode::output,CM>;
}

#endif
