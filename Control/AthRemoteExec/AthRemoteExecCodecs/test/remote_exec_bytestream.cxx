/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file remote_exec_bytestream.cxx
 * @brief The bytestream wire boundary: toBytes()/fromBytes() round trip and
 * every error path.
 *
 * A valid full event is built with eformat::write and then corrupted in every
 * way the conversion layer is supposed to catch -- one corruption per
 * Error.
 */

#include <AthRemoteExecCodecs/Bytestream.h>
#include <eformat/Status.h>
#include <eformat/write/FullEventFragment.h>
#include <eformat/write/ROBFragment.h>
#include <eformat/write/node.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "CxxUtils/checker_macros.h"

// Single-threaded by construction.
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

namespace {

int s_failures = 0;

void check(bool condition, const std::string& what) {
  if (!condition) {
    std::cout << std::format("FAIL: {}\n", what);
    ++s_failures;
  } else {
    std::cout << std::format("ok: {}\n", what);
  }
}

/// A small but complete full event: one ROB carrying two data words.
std::vector<uint32_t> makeEvent() {
  const std::vector<uint32_t> rodData = {0x11223344, 0x55667788};
  eformat::write::ROBFragment rob(0x00000011,  // ROB source id
                                  456789,      // run number
                                  4242,        // LVL1 id
                                  1234,        // bunch crossing id
                                  0x1a,        // LVL1 trigger type
                                  0,           // detector event type
                                  rodData.size(), rodData.data(), eformat::STATUS_FRONT);
  eformat::write::FullEventFragment fe(0x00000001,     // source id
                                       1700000000, 0,  // bunch crossing time
                                       0xdeadbeef,     // global id
                                       0,              // run type
                                       456789,         // run number
                                       12,             // luminosity block
                                       4242,           // LVL1 id
                                       1234,           // bunch crossing id
                                       0x1a);          // LVL1 trigger type
  fe.append(&rob);

  std::vector<uint32_t> words(fe.size_word());
  eformat::write::copy(*fe.bind(), words.data(), words.size());
  return words;
}

std::string asBytes(const std::vector<uint32_t>& words) {
  return std::string(reinterpret_cast<const char*>(words.data()), words.size() * sizeof(uint32_t));
}

}  // anonymous namespace

int main() {
  using namespace AthRemoteExec::Bytestream;

  const std::vector<uint32_t> words = makeEvent();

  // -- toBytes, success
  const RawEvent event(words.data());
  auto bytes = toBytes(&event);
  check(bytes.has_value(), "toBytes() encodes a valid event");
  check(bytes && bytes->size() == words.size() * sizeof(uint32_t), "toBytes() yields one byte per word");

  // -- toBytes, error paths
  auto nullBytes = toBytes(nullptr);
  check(!nullBytes && nullBytes.error().error == Error::NullEvent, "toBytes() rejects a null event");

  const RawEvent empty;
  auto emptyBytes = toBytes(&empty);
  check(!emptyBytes && emptyBytes.error().error == Error::EmptyEvent, "toBytes() rejects an empty event");

  // -- fromBytes, error paths: one corruption per Error
  auto emptyPayload = fromBytes("");
  check(!emptyPayload && emptyPayload.error().error == Error::EmptyPayload, "fromBytes() rejects an empty payload");

  auto misaligned = fromBytes(std::string(3, '\0'));
  check(!misaligned && misaligned.error().error == Error::MisalignedPayload,
        "fromBytes() rejects sizes that don't align to the word size");

  auto tooShort = fromBytes(std::string(12, '\0'));
  check(!tooShort && tooShort.error().error == Error::BadMarker, "fromBytes() rejects buffers shorter than a header");

  auto noMarker = fromBytes(std::string(32, '\x5a'));
  check(!noMarker && noMarker.error().error == Error::BadMarker, "fromBytes() rejects a wrong marker");

  {
    auto corrupt = words;
    corrupt[1] += 1;  // declared fragment size
    auto res = fromBytes(asBytes(corrupt));
    check(!res && res.error().error == Error::TruncatedFragment, "fromBytes() rejects an incorrect size word");
  }
  {
    const std::string full = asBytes(words);
    auto res = fromBytes(std::string(full.data(), full.size() - 4));
    check(!res && res.error().error == Error::TruncatedFragment, "fromBytes() rejects a truncated transfer");
  }
  {
    auto corrupt = words;
    corrupt.push_back(0);  // trailing padding
    auto res = fromBytes(asBytes(corrupt));
    check(!res && res.error().error == Error::TruncatedFragment, "fromBytes() rejects trailing padding");
  }
  {
    auto corrupt = words;
    corrupt[2] = static_cast<uint32_t>(corrupt.size()) + 1;  // header size
    auto res = fromBytes(asBytes(corrupt));
    check(!res && res.error().error == Error::InvalidFragment, "fromBytes() rejects a structurally broken header");
  }

  // -- round trip
  auto back = fromBytes(*bytes);
  check(back.has_value(), "fromBytes() accepts what toBytes() produced");
  if (back) {
    check((*back)->words() == words, "Round trip words are identical");
    check((*back)->event()->run_no() == 456789u, "Run number survives");
    check((*back)->event()->lvl1_id() == 4242u, "L1 id survives");
    check((*back)->event()->bc_id() == 1234u, "Bunch crossing id survives");
    check((*back)->event()->lvl1_trigger_type() == 0x1a, "L1 type survives");

    auto iter = (*back)->event()->child_iter();
    size_t robs = 0;
    while (iter.next() != nullptr) {
      ++robs;
    }
    check(robs == 1, "The decoded event still has one ROB");
  }

  if (s_failures != 0) {
    std::cout << std::format("remote_exec_bytestream: {} check(s) failed\n", s_failures);
    return EXIT_FAILURE;
  }

  std::cout << "remote_exec_bytestream: all checks passed\n";
  return EXIT_SUCCESS;
}
