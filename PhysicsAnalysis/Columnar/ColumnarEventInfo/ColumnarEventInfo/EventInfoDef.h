/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EVENT_INFO_EVENT_INFO_DEF_H
#define COLUMNAR_EVENT_INFO_EVENT_INFO_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <xAODEventInfo/EventInfo.h>

namespace columnar
{
  template<> struct ContainerIdTraits<ContainerId::eventInfo> final
  {
    static constexpr bool isDefined = true;
    static constexpr bool isMutable = false;
    static constexpr bool perEventRange = false;
    static constexpr bool perEventId = true;

    /// the xAOD type to use with ObjectId
    using xAODObjectIdType = const xAOD::EventInfo;

    /// the xAOD type to use with ObjectRange
    using xAODObjectRangeType = const xAOD::EventInfo;

    /// the xAOD type to use with ElementLink
    using xAODElementLinkType = xAOD::EventInfo;
  };
}

#endif
