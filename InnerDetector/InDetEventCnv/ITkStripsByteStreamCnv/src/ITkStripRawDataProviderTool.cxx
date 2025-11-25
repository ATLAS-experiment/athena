/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ITkStripRawDataProviderTool.h"

#include "ITkStripsByteStreamCnv/IITkStripsRodDecoder.h"
#include "StoreGate/ReadHandle.h"

using OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;

// Initialize
StatusCode ITkStripRawDataProviderTool::initialize()
{
  ATH_CHECK(m_decoder.retrieve());

  return StatusCode::SUCCESS;
}


// Convert method
StatusCode ITkStripRawDataProviderTool::convert(std::vector<const ROBFragment*>& vecROBFrags,
                                           SCT_RDO_Container& rdoIDCont,
                                           IDCInDetBSErrContainer& errs,
                                           DataPool<SCT3_RawData>* dataItemsPool,
                                           const EventContext& ctx) const
{
  
  ATH_MSG_DEBUG("ITkStripRawDataProviderTool::convert()");
  if (vecROBFrags.empty()) return StatusCode::SUCCESS;
  // loop over the ROB fragments
  StatusCode sc{StatusCode::SUCCESS};
  ATH_MSG_DEBUG("vecROBFrags size: " << vecROBFrags.size());
  for (const ROBFragment* robFrag : vecROBFrags) {
    // get the ID of this ROB/ROD
    sc = m_decoder->fillCollection(*robFrag, rdoIDCont, errs, dataItemsPool, ctx);
    if (sc == StatusCode::FAILURE) {
      if (m_decodeErrCount <= 100) {
        if (100 == m_decodeErrCount) {
          ATH_MSG_ERROR("Too many Problem with ITk Strip Decoding messages, turning message off.");
        }
        else {
          ATH_MSG_ERROR("Problem with ITk Strip ByteStream Decoding!");
        }
        m_decodeErrCount++;
      }
    }
  }

  if (sc == StatusCode::FAILURE) {
    ATH_MSG_ERROR("There was a problem with ITk Strip ByteStream conversion");
    return sc;
  }

  return sc;
}
