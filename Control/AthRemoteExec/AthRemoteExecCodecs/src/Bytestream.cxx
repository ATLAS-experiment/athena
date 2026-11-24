/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <AthRemoteExecCodecs/Bytestream.h>
#include <eformat/HeaderMarker.h>

#include <cstring>
#include <format>
#include <utility>
#include <vector>

namespace AthRemoteExec::Bytestream {

namespace { constexpr std::size_t wordSize = sizeof(uint32_t); }

std::expected<std::string, Failure> toBytes(const RawEvent* event) {
  if (event == nullptr) {
    return std::unexpected(Failure{Error::NullEvent, "RawEvent is null!"});
  }

  if (event->start() == nullptr) {
    return std::unexpected(Failure{Error::EmptyEvent, "RawEvent is empty; nothing to encode"});
  }

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
