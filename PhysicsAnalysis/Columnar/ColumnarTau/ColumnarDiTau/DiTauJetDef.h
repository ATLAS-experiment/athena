/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_DITAU_DITAU_JET_DEF_H
#define COLUMNAR_DITAU_DITAU_JET_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODTau/DiTauJetContainer.h>

namespace columnar
{
  struct DiTauJetDef : RegularContainerId<xAOD::DiTauJet,xAOD::DiTauJetContainer>
  {
    static constexpr std::string_view idName = "ditauJet";
  };

  using DiTauJetRange = ObjectRange<DiTauJetDef>;
  using DiTauJetId = ObjectId<DiTauJetDef>;
  using OptDiTauJetId = OptObjectId<DiTauJetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using DiTauJetAccessor  = AccessorTemplate<DiTauJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using DiTauJetDecorator = AccessorTemplate<DiTauJetDef,CT,ColumnAccessMode::output,CM>;
}

#endif
