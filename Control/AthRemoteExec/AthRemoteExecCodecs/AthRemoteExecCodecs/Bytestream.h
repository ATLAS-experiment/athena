/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCODECS_BYTESTREAM_H
#define ATHREMOTEEXECCODECS_BYTESTREAM_H

#include <AthRemoteExecCodecs/RawEventCache.h>
#include <eformat/SourceIdentifier.h>

#include <expected>
#include <memory>
#include <set>
#include <string>
#include <string_view>

namespace AthRemoteExec::Bytestream {

/// Why @c toBytes or @c fromBytes failed. Meant for test assertions;
enum class Error {
  NullEvent,          // toBytes: no event to encode
  EmptyEvent,         // toBytes: the event carries no data
  NoMatchingRobs,     // toBytes: the detector selection matched no ROB
  EmptyPayload,       // fromBytes: zero bytes received
  MisalignedPayload,  // fromBytes: size is not a multiple of the word size
  BadMarker,          // fromBytes: doesn't start with a FullEventFragment
  TruncatedFragment,  // fromBytes: declared size differs from received size
  InvalidFragment,    // fromBytes: failed eformat validation
};

struct Failure {
  Error error = Error::NullEvent;
  std::string diagnostics;  ///< human-readable specifics, for log messages
};

/// Encode a RawEvent into wire bytes. If RawEvent is invalid (null or has no data) it will fail here. If @c keep is
/// non-empty, the event is rebuilt with only the ROBs whose source identifier's sub-detector groups are listed – event
/// here is uncompressed.
[[nodiscard]] std::expected<std::string, Failure> toBytes(const RawEvent* event,
                                                          const std::set<eformat::SubDetectorGroup>& keep = {});

/// Decode wire bytes into a fully validated RawEventCache. The received bytes are checked for size and eformat
/// structure before anything reads into them.
[[nodiscard]] std::expected<std::unique_ptr<RawEventCache>, Failure> fromBytes(std::string_view bytes);

}  // namespace AthRemoteExec::Bytestream

#endif  // ATHREMOTEEXECCODECS_BYTESTREAM_H
