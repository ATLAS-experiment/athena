/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "eformat_utils.h"
#include <vector>
#include <set>

#include "eformat/eformat.h"
#include "eformat/index.h"
#include "eformat/write/eformat.h"
#include "eformat/SourceIdentifier.h"
#include "eformat/FullEventFragmentNoTemplates.h"
#include "eformat/compression.h"

ERS_DECLARE_ISSUE(HLTMPPUIssue,   // namespace name
		  EformatIssue,   // issue name
		  "Eformat Issue: \"" << type << "\".",        // message
		  ((const char *)type )             // first attribute
		  )

std::vector<uint32_t>
  DFEF::merge_hltresult_with_input_event(const uint32_t* hltresult,
                                          const uint32_t* input,
                                          eformat::Compression comp,
                                          unsigned int comp_level) {
  ERS_DEBUG(5, "Create read fullevent");
  eformat::read::FullEventFragment fullevent(input);  // Input full event
  ERS_DEBUG(5, "Create write hltresult");
  eformat::write::FullEventFragment newEvent(hltresult);  // HLT result robs and event header

  // Get stream tag from event passed by HLT
  auto nstream_tag = newEvent.nstream_tag();
  auto stream_tag = newEvent.stream_tag();
  ERS_DEBUG(4, nstream_tag  << " stream tag words, stream_tag[0]= " << stream_tag[0]);
  std::vector<eformat::helper::StreamTag> vstream_tag;
  eformat::helper::decode(nstream_tag, stream_tag, vstream_tag);

  // set compression type
  newEvent.compression_type(comp);
  newEvent.compression_level(comp_level);
  newEvent.lvl2_trigger_info(0, nullptr);

  std::vector<eformat::read::ROBFragment> currRobs;
  fullevent.robs(currRobs);
  ERS_DEBUG(4, currRobs.size() << " robs in input event");

  // Create a set of all ROB IDs in the HLT Result event
  eformat::read::FullEventFragment tempEvent(hltresult);
  std::vector<eformat::read::ROBFragment> tempRobs;
  tempEvent.robs(tempRobs);
  std::set<uint32_t> hltResultRobIds;
  for (auto & r : tempRobs) {
    hltResultRobIds.emplace(r.rob_source_id());
  }

  std::set<uint32_t> robset;
  std::set<eformat::SubDetector> detset;
  for (const auto &t : vstream_tag) {
    if (t.robs.empty()&& t.dets.empty()) {  // At least one stream_tag require full event building
      robset.clear();
      detset.clear();
      break;
    } else {  // Only add the ROBs requested by stream tags
      std::copy(std::begin(t.robs), std::end(t.robs),
                std::inserter(robset, std::begin(robset)));
      std::copy(std::begin(t.dets), std::end(t.dets),
                std::inserter(detset, std::begin(detset)));
    }
  }

  std::vector<eformat::write::ROBFragment> robs2write;
  robs2write.reserve(currRobs.size());

  // Function that decides which ROBs to add to new event
  auto toCopy = [&](const auto & rob){
    auto sId = rob.rob_source_id();
    auto subId = eformat::helper::SourceIdentifier{sId}.subdetector_id();

    bool build_full_event = robset.empty() && detset.empty();
    bool HLT_rob = (subId == eformat::TDAQ_HLT);
    bool already_in_HLTResult = (hltResultRobIds.find(sId) != hltResultRobIds.end());
    bool suitable_for_copy = (!HLT_rob && !already_in_HLTResult);

    if (build_full_event && suitable_for_copy) {  // 1. Full event building
      return true;
    } else {  // Partial event building with only some robs
      bool requested_by_streamtag = (robset.find(sId) != robset.end()|| detset.find(subId) != detset.end());
      if (requested_by_streamtag && suitable_for_copy) {
        return true;
      }
    }
    return false;
  };

  for (const auto& r : currRobs) {
    if (toCopy(r)) {
      robs2write.push_back(r.start());
    }
  }

  ERS_DEBUG(4, robs2write.size() << " ROBs will be written to write event");
  // do I need to do that?
  for (size_t t = 0; t < robs2write.size(); t++) {
    newEvent.append(&robs2write[t]);
  }
  // compression happens here
  const eformat::write::node_t* top = newEvent.bind();
  auto finalSize = newEvent.size_word();
  auto finalEvent = std::make_unique<uint32_t[]>(finalSize);
  ERS_DEBUG(4, finalSize << " words will be copied to final event");
  auto res = eformat::write::copy(*top, finalEvent.get(), finalSize);
  if (res != finalSize) {
    std::string errMsg("ERROR, event serialization failed - ");
    errMsg += std::string("Serialized event size: ") + std::to_string(finalSize);
    errMsg += std::string(", Written size: ") + std::to_string(res);
    auto issue = HLTMPPUIssue::EformatIssue(ERS_HERE,errMsg.c_str());
    ers::warning(issue);
    throw issue;
  }
  std::vector<uint32_t> result;
  result.assign(finalEvent.get(), finalEvent.get() + finalSize);
  return result;
}
