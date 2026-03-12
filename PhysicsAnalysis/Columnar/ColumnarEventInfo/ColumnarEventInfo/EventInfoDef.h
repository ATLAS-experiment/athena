/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EVENT_INFO_EVENT_INFO_DEF_H
#define COLUMNAR_EVENT_INFO_EVENT_INFO_DEF_H

#include <ColumnarCore/ContainerId.h>
#include <ColumnarInterfaces/ColumnarDef.h>
#include <xAODEventInfo/EventInfo.h>

namespace columnar
{
  namespace ContainerId
  {
    struct eventInfo : regularCIBase<xAOD::EventInfo,xAOD::EventInfo>
    {
      // The `idName` is used to identify the container internally, but
      // it is also the fallback if the tool doesn't explicitly define a
      // name for the object. So I defined it as "EventInfo" which is
      // almost always the name used in the input file.
      static constexpr std::string_view idName = "EventInfo";

      // redefine this to be per-event ObjectId instead of per-event
      // ObjectRange
      static constexpr bool perEventRange = false;
      static constexpr bool perEventId = true;
    };
  }

  using EventInfoRange = ObjectRange<ContainerId::eventInfo>;
  using EventInfoId = ObjectId<ContainerId::eventInfo>;
  using OptEventInfoId = OptObjectId<ContainerId::eventInfo>;
  template<typename CT,typename CM=ColumnarModeDefault> using EventInfoAccessor  = AccessorTemplate<ContainerId::eventInfo,CT,ColumnAccessMode::input,CM>;
  template<typename CT,typename CM=ColumnarModeDefault> using EventInfoDecorator = AccessorTemplate<ContainerId::eventInfo,CT,ColumnAccessMode::output,CM>;
}

#endif
