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

  template<ColumnarMode CM> using TauJetRange = ObjectRange<TauJetDef,CM>;
  template<ColumnarMode CM> using TauJetId = ObjectId<TauJetDef,CM>;
  template<ColumnarMode CM> using OptTauJetId = OptObjectId<TauJetDef,CM>;
  template<typename CT,ColumnarMode CM> using TauJetAccessor  = AccessorTemplate<TauJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using TauJetDecorator = AccessorTemplate<TauJetDef,CT,ColumnAccessMode::output,CM>;
}

#endif
