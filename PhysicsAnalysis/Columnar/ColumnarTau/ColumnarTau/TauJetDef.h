/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_TAU_TAU_JET_DEF_H
#define COLUMNAR_TAU_TAU_JET_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODTau/TauJetContainer.h>

namespace columnar
{
  struct TauJetDef : RegularContainerId<xAOD::TauJet,xAOD::TauJetContainer>
  {
    static constexpr std::string_view idName = "tauJet";
  };

  using TauJetRange = ObjectRange<TauJetDef, ColumnarModeDefault>;
  using TauJetId = ObjectId<TauJetDef>;
  using OptTauJetId = OptObjectId<TauJetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using TauJetAccessor  = AccessorTemplate<TauJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using TauJetDecorator = AccessorTemplate<TauJetDef,CT,ColumnAccessMode::output,CM>;
}

#endif
