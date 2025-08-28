/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PadTrig_ROD_Decoder.h"

#include <byteswap.h>
#include <vector>

#include "Identifier/Identifier.h"
#include "eformat/Issue.h"
#include "eformat/SourceIdentifier.h"
#include "MuonNSWCommonDecode/NSWTriggerCommonDecoder.h"
#include "MuonNSWCommonDecode/NSWPadTriggerL1a.h"

namespace Muon {

//=====================================================================
PadTrig_ROD_Decoder::PadTrig_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent)
: AthAlgTool(type, name, parent)
{
  declareInterface<IPadTrig_ROD_Decoder>(this);
}


//=====================================================================
StatusCode PadTrig_ROD_Decoder::fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, NSW_PadTriggerDataContainer& rdo) const
{
  try {
    fragment.check();
  } catch (const eformat::Issue& ex) {
    ATH_MSG_ERROR(ex.what());
    return StatusCode::FAILURE;
  }

  const std::string trigger{"PadL1A"};
  const Muon::nsw::NSWTriggerCommonDecoder decoder{fragment, trigger};
  for (const auto& baselink: decoder.get_elinks()) {
    const auto link = static_cast<const Muon::nsw::NSWPadTriggerL1a*>(baselink.get());
    auto *const collection = new NSW_PadTriggerData{
      fragment.rob_source_id(),
      link->getFlags(),
      link->getEc(),
      link->getFragid(),
      link->getSecid(),
      link->getSpare(),
      link->getOrbit(),
      link->getBcid(),
      link->getL1id(),
      link->getOrbitid(),
      link->getOrbit1(),
      link->getStatus(),
      link->getNumberOfHits(),
      link->getNumberOfPfebs(),
      link->getNumberOfTriggers(),
      link->getNumberOfBcids(),
      link->getHitRelBcids(),
      link->getHitPfebs(),
      link->getHitTdsChannels(),
      link->getHitVmmChannels(),
      link->getHitVmms(),
      link->getHitPadChannels(),
      link->getPfebAddresses(),
      link->getPfebNChannels(),
      link->getPfebDisconnecteds(),
      link->getTriggerBandIds(),
      link->getTriggerPhiIds(),
      link->getTriggerRelBcids(),
      link->getBcidRels(),
      link->getBcidStatuses(),
      link->getBcidMultZeros(),
      link->getBcidMultiplicities()
    };
    ATH_MSG_DEBUG("Pad trigger fillCollection " << std::hex << collection->getSourceid() << std::dec << " " << rdo.numberOfCollections());
    ATH_CHECK(rdo.addCollection(collection, rdo.numberOfCollections()));
  }

  return StatusCode::SUCCESS;
}

}  // namespace Muon
