// -*- c++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKSYSTEMATICSTOOLS_GETEVENTNUMBER_H
#define INDETTRACKSYSTEMATICSTOOLS_GETEVENTNUMBER_H

#ifndef XAOD_STANDALONE
#include <GaudiKernel/EventContext.h>
#include <GaudiKernel/ThreadLocalContext.h>
#else
#include "xAODEventInfo/EventInfo.h"
#include <stdexcept>
#endif

#include <cstdint>

namespace InDet {

/// Return the event number for the currently-executing event.
///
/// In Athena, reads the thread-local EventContext with zero StoreGate
/// overhead.  In standalone, retrieves xAOD::EventInfo from the store.
/// Throws std::runtime_error if the event number cannot be determined.
template<typename Store>
inline uint64_t getEventNumber([[maybe_unused]] Store&& store) {
#ifndef XAOD_STANDALONE
    return Gaudi::Hive::currentContext().eventID().event_number();
#else
    const xAOD::EventInfo* ei = nullptr;
    if (!store || !store->retrieve(ei, "EventInfo").isSuccess() || !ei)
        throw std::runtime_error(
            "getEventNumber: failed to retrieve EventInfo");
    return ei->eventNumber();
#endif
}

} // namespace InDet

#endif
