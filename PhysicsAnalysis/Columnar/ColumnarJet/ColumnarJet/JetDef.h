/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_JET_JET_DEF_H
#define COLUMNAR_JET_JET_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODJet/JetContainer.h>

namespace columnar
{
  struct JetDef : RegularContainerId<xAOD::Jet,xAOD::JetContainer>
  {
    static constexpr std::string_view idName = "jet";
  };
  using MutableJetDef = MutableContainerId<JetDef>;

  template<ColumnarMode CM> using JetRange = ObjectRange<JetDef,CM>;
  template<ColumnarMode CM> using JetId = ObjectId<JetDef,CM>;
  template<ColumnarMode CM> using OptJetId = OptObjectId<JetDef,CM>;
  template<typename CT,ColumnarMode CM> using JetAccessor  = AccessorTemplate<JetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using JetDecorator = AccessorTemplate<JetDef,CT,ColumnAccessMode::output,CM>;

  template<ColumnarMode CM> using MutableJetRange = ObjectRange<MutableJetDef,CM>;
  template<ColumnarMode CM> using MutableJetId = ObjectId<MutableJetDef,CM>;
  template<ColumnarMode CM> using OptMutableJetId = OptObjectId<MutableJetDef,CM>;
  template<typename CT,ColumnarMode CM> using MutableJetAccessor  = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,ColumnarMode CM> using MutableJetDecorator = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::output,CM>;
  template<typename CT,ColumnarMode CM> using MutableJetUpdater = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::update,CM>;
}

#endif
