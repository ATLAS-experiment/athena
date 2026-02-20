/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAOD_ANALYSIS

#ifndef BYTESTREAMDATA_RAWEVENT_H
#define BYTESTREAMDATA_RAWEVENT_H

//
//  Initial version:  July 10, 2002
//        typedef for RawEvent from eformat.
//---------------------------------------------------------------------

#include <cstdint>

#include "eformat/FullEventFragment.h"
#include "eformat/ROBFragment.h"
#include "eformat/write/FullEventFragment.h"
#include "eformat/write/ROBFragment.h"

namespace OFFLINE_FRAGMENTS_NAMESPACE {
  /*@name type aliases to read fragments */
  using DataType = uint32_t;
  using PointerType = const DataType*;
  using FullEventFragment = eformat::FullEventFragment<PointerType>;
  using ROBFragment = eformat::ROBFragment<PointerType>;
}

namespace OFFLINE_FRAGMENTS_NAMESPACE_WRITE {
  /*@name type aliases to write fragments */
  using FullEventFragment = eformat::write::FullEventFragment;
  using ROBFragment = eformat::write::ROBFragment;
}

/// data type for reading raw event
using RawEvent = OFFLINE_FRAGMENTS_NAMESPACE::FullEventFragment;
/// data type for writing raw event
using RawEventWrite = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::FullEventFragment;

#endif

#endif
