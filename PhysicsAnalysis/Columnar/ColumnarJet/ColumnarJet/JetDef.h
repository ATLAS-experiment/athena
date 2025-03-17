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
  template<> struct ContainerIdTraits<ContainerId::jet> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::Jet;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::JetContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::JetContainer;
  };

  template<> struct ContainerIdTraits<ContainerId::mutableJet> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = true;
    static constexpr ContainerId constId = ContainerId::jet;
    static constexpr bool perEventRange = true;
    static constexpr bool perEventId = false;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = xAOD::Jet;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = xAOD::JetContainer;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::JetContainer;
  };
}

#endif
