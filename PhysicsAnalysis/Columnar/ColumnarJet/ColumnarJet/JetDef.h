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
  namespace ContainerId
  {
    struct jet : regularCIBase<xAOD::Jet,xAOD::JetContainer>
    {
      static constexpr std::string_view idName = "jet";
    };
    using mutableJet = mutableCI<jet>;
  }

  using JetRange = ObjectRange<ContainerId::jet>;
  using JetId = ObjectId<ContainerId::jet>;
  using OptJetId = OptObjectId<ContainerId::jet>;
  template<typename CT,typename CM=ColumnarModeDefault> using JetAccessor  = AccessorTemplate<ContainerId::jet,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using JetDecorator = AccessorTemplate<ContainerId::jet,CT,ColumnAccessMode::output,CM>;

  using MutableJetRange = ObjectRange<ContainerId::mutableJet>;
  using MutableJetId = ObjectId<ContainerId::mutableJet>;
  using OptMutableJetId = OptObjectId<ContainerId::mutableJet>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetAccessor  = AccessorTemplate<ContainerId::mutableJet,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetDecorator = AccessorTemplate<ContainerId::mutableJet,CT,ColumnAccessMode::output,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using MutableJetUpdater = AccessorTemplate<ContainerId::mutableJet,CT,ColumnAccessMode::update,CM>;
}

#endif
