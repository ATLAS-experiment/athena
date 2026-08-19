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

  template<ColumnarMode CM> using DiTauJetRange = ObjectRange<DiTauJetDef,CM>;
  template<ColumnarMode CM> using DiTauJetId = ObjectId<DiTauJetDef,CM>;
  template<ColumnarMode CM> using OptDiTauJetId = OptObjectId<DiTauJetDef,CM>;
  template<typename CT,ColumnarMode CM> using DiTauJetAccessor  = AccessorTemplate<DiTauJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using DiTauJetDecorator = AccessorTemplate<DiTauJetDef,CT,ColumnAccessMode::output,CM>;
}

#endif
