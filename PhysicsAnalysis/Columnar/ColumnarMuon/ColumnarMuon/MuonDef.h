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
  template<> struct ContainerIdTraits<ContainerId::muon> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Muon;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::MuonContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::MuonContainer;
  };
}

#endif
