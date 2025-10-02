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
  namespace ContainerId
  {
    struct eventInfo : regularCIBase<xAOD::EventInfo,xAOD::EventInfo>
    {
      // this is hard-coded in the ColumnarTool implementation, if you
      // change it here, you need to change it there as well
      static constexpr std::string_view idName = "eventInfo";

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
