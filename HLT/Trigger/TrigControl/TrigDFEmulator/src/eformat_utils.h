/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGDFEMULATOR_EFORMAT_UTILS_H
#define TRIGDFEMULATOR_EFORMAT_UTILS_H

#include <vector>
#include <cstdint>
#include "eformat/compression.h"

namespace DFEF {

//! Merge two events: Take header and robs from hltresult event, add all res
//! of the ROBS from input event
//! \param hltresult Pointer to serialized FullEventFragment filled by HLT,
//!   contains a full header, HLT result ROBs and possibly simulated L1 ROBs 
//! \param input Pointer to serialized FullEventFragment, which was input for
//!   event processing.
//! \param comp Compression type
//! \param comp_level Compression Level
//! \return merged_event vector that holds the serialized FullEventFragment
std::vector<uint32_t>
    merge_hltresult_with_input_event(const uint32_t* hltresult,
                                     const uint32_t* input,
                                     eformat::Compression comp,
                                     unsigned int comp_level);
}  // namespace DFEF

#endif // TRIGDFEMULATOR_EFORMAT_UTILS_H
