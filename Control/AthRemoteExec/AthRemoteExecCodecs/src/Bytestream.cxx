/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCodecs/Bytestream.h>
#include <eformat/HeaderMarker.h>
#include <eformat/SourceIdentifier.h>
#include <eformat/compression.h>
#include <eformat/write/FullEventFragment.h>
#include <eformat/write/ROBFragment.h>
#include <eformat/write/node.h>

#include <cstring>
#include <deque>
#include <format>
#include <set>
#include <utility>
#include <vector>

namespace AthRemoteExec::Bytestream {

namespace { constexpr std::size_t wordSize = sizeof(uint32_t); }

std::expected<std::string, Failure> toBytes(const RawEvent* event, const std::set<eformat::SubDetectorGroup>& keep) {
  if (event == nullptr) {
    return std::unexpected(Failure{Error::NullEvent, "RawEvent is null!"});
  }

  if (event->start() == nullptr) {
    return std::unexpected(Failure{Error::EmptyEvent, "RawEvent is empty; nothing to encode"});
  }

  // No selection -> copy entire event
  if (keep.empty()) {
    const std::size_t nBytes = std::distance(event->start(), event->end()) * wordSize;
    if (nBytes == 0) {
      return std::unexpected(Failure{Error::EmptyEvent, "RawEvent is empty; nothing to encode"});
    }

    std::string bytes;
    bytes.resize_and_overwrite(nBytes, [&](char* dest, std::size_t count) {
      std::memcpy(dest, event->start(), count);
      return count;
    });

    return bytes;
  }

  // Rebuild event using only matching sub-detectors
  eformat::write::FullEventFragment full;
  full.copy_header(event->start());
  full.compression_type(eformat::UNCOMPRESSED);

  // To keep fragmens addresses stable until fragments are written
  std::deque<eformat::write::ROBFragment> wrappers;
  std::size_t nSelected = 0;
  for (auto it = event->child_iter(); const uint32_t* fp = it.next();) {
    const eformat::helper::SourceIdentifier sid(eformat::ROBFragment<const uint32_t*>(fp).source_id());
    if (!keep.contains(sid.subdetector_group())) {
      continue;
    }

    wrappers.emplace_back(fp);
    full.append(&wrappers.back());
    ++nSelected;
  }

  if (nSelected == 0) {
    return std::unexpected(Failure{Error::NoMatchingRobs, "No ROB of the requested detectors found in the event"});
  }

  const uint32_t nWords = full.size_word();
  std::vector<uint32_t> words(nWords);
  eformat::write::copy(*full.bind(), words.data(), nWords);

  std::string bytes;
  bytes.resize_and_overwrite(nWords * wordSize, [&](char* dest, std::size_t count) {
    std::memcpy(dest, words.data(), count);
    return count;
  });

  return bytes;
}

std::expected<std::unique_ptr<RawEventCache>, Failure> fromBytes(std::string_view bytes) {
  if (bytes.empty()) {
    return std::unexpected(Failure{Error::EmptyPayload, "Received an empty bytestream payload"});
  }

  if (bytes.size() % wordSize != 0) {
    return std::unexpected(Failure{Error::MisalignedPayload, std::format("Payload size ({} bytes) is not a multiple "
                                                                         "of the word size ({} bytes)",
                                                                         bytes.size(), wordSize)});
  }

  const std::size_t nWords = bytes.size() / wordSize;
  std::vector<uint32_t> words(nWords);
  std::memcpy(words.data(), bytes.data(), bytes.size());

  if (nWords < 4 || words[0] != static_cast<uint32_t>(eformat::FULL_EVENT)) {
    return std::unexpected(Failure{
        Error::BadMarker, std::format("Payload of {} words doesn't start with a FullEventFragment marker", nWords)});
  }

  if (words[1] != nWords) {
    return std::unexpected(
        Failure{Error::TruncatedFragment,
                std::format("FullEventFragment declares {} words but only {} were received", words[1], nWords)});
  }

  auto cache = std::make_unique<RawEventCache>(std::move(words));
  if (!cache->event()->check_tree_noex()) {
    return std::unexpected(Failure{Error::InvalidFragment, "Bytestream payload failed eformat validation"});
  }

  return cache;
}

}  // namespace AthRemoteExec::Bytestream
