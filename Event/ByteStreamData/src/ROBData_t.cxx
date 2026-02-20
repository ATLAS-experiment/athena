/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef XAOD_ANALYSIS
#include "ByteStreamData/ROBData.h"
#include "ByteStreamData/RawEvent.h"

// Test at compiler time ROBData class

namespace ByteStreamDataTest {

  void ROBDataT(OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment* robFrag) {

    using IT = ROBData::iterator;

    ROBData* robData = new ROBData(robFrag);
    ROBData robData2 = *robData;

    for (IT it = robData->begin(), it_e = robData->end(); it != it_e; ++it) {
      [[maybe_unused]] uint32_t i = (*it);
      ++i;
    }

    robData->sub_detector();
    robData->module_identifier();
    robData2.sub_detector();

    delete robData;
    robData = nullptr;
    return;
  }
}
#endif
