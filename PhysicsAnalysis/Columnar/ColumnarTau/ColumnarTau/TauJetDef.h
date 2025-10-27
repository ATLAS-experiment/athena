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
  namespace ContainerId
  {
    struct tauJet : regularCIBase<xAOD::TauJet,xAOD::TauJetContainer>
    {
      static constexpr std::string_view idName = "tauJet";
    };
  }

  using TauJetRange = ObjectRange<ContainerId::tauJet>;
  using TauJetId = ObjectId<ContainerId::tauJet>;
  using OptTauJetId = OptObjectId<ContainerId::tauJet>;
  template<typename CT,typename CM=ColumnarModeDefault> using TauJetAccessor  = AccessorTemplate<ContainerId::tauJet,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using TauJetDecorator = AccessorTemplate<ContainerId::tauJet,CT,ColumnAccessMode::output,CM>;
}

#endif
