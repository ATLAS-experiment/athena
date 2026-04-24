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

  using JetRange = ObjectRange<JetDef>;
  using JetId = ObjectId<JetDef>;
  using OptJetId = OptObjectId<JetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using JetAccessor  = AccessorTemplate<JetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using JetDecorator = AccessorTemplate<JetDef,CT,ColumnAccessMode::output,CM>;

  using MutableJetRange = ObjectRange<MutableJetDef>;
  using MutableJetId = ObjectId<MutableJetDef>;
  using OptMutableJetId = OptObjectId<MutableJetDef>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetAccessor  = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetDecorator = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::output,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetUpdater = AccessorTemplate<MutableJetDef,CT,ColumnAccessMode::update,CM>;
}

#endif
