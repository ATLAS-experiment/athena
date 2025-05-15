/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef COLUMNAR_EVENT_INFO_EVENT_INFO_HELPERS_H
#define COLUMNAR_EVENT_INFO_EVENT_INFO_HELPERS_H

#include <ColumnarCore/ColumnAccessor.h>
#include <ColumnarEventInfo/EventInfoDef.h>

namespace columnar
{
  namespace EventInfoHelpers
  {
    /// @file accessor for variables that have calculations in @ref xAOD::EventInfo
    ///
    /// Essentially this just copies out the relevant parts of the xAOD
    /// class and makes them look like stand-alone accessors.  The name
    /// of each class is derived from the member function in the xAOD
    /// class.


    template<ContainerId CI = ContainerId::eventInfo,typename CM=ColumnarModeDefault>
    class EventTypeAccessor final
    {
      ColumnAccessor<CI,uint32_t,CM> m_eventTypeBitmaskAcc;

    public:
      
      EventTypeAccessor (ColumnarTool<CM>& columnarTool)
        : m_eventTypeBitmaskAcc (columnarTool, "eventTypeBitmask") {}

      bool operator () (ObjectId<CI,CM> object, xAOD::EventInfo::EventType type) const
      {
        return m_eventTypeBitmaskAcc(object) & static_cast< uint32_t >( type );
      }
    };
  }
}

#endif
