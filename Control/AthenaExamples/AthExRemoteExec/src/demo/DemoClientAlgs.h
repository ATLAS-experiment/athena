/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_DEMOCLIENTALGS_H
#define ATHEXREMOTEEXEC_DEMOCLIENTALGS_H

/**
 * @file DemoClientAlgs.h
 * @brief What an Athena client of the demonstration fragment does either side
 *        of the call.
 *
 * Neither of these knows an RemoteExec is involved, which is the claim being made:
 * @c DemoNumbersAlg writes ordinary integers into the event store and
 * @c DemoCheckAlg reads ordinary integers out of it, and the same two would
 * work unchanged either side of a local SumAlg. Everything between them --
 * DemoIntsPackAlg, RemoteExecRequestAlg, DemoIntsUnpackAlg -- is configuration.
 *
 * DemoNumbersAlg also serves the input-free RemoteExecSeqPing fragment on the server,
 * which is the same observation from the other end: an algorithm that makes up
 * numbers does not care which side of a boundary it is on.
 *
 * DemoCheckAlg exists because a reply that decoded but arrived empty returns a
 * clean StatusCode. Checking the numbers is the only way to tell "the server
 * answered" from "the server answered correctly", and it is what makes the
 * client test an assertion rather than a smoke test.
 */

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include <atomic>
#include <cstdint>
#include <vector>

namespace AthExRemoteExec {

/// Makes up the numbers a client sends: one per declared key, offset by the
/// event number so that no two events send the same request.
class DemoNumbersAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  SG::WriteHandleKeyArray<int64_t> m_values{this, "Values", {},
                                            "One key per number to produce"};
  Gaudi::Property<std::vector<int>> m_offsets{
      this, "Offsets", {},
      "Per key, what to add to the event number. Must be as long as Values"};
};

/// Fails the job unless Result is the sum of Values plus Offset.
class DemoCheckAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;
  virtual StatusCode finalize() override;

private:
  SG::ReadHandleKeyArray<int64_t> m_values{this, "Values", {},
                                           "The numbers that were sent"};
  SG::ReadHandleKey<int64_t> m_result{this, "Result", "",
                                      "What came back"};
  Gaudi::Property<int> m_offset{
      this, "Offset", 0, "Constant the fragment is expected to have added"};

  mutable std::atomic<size_t> m_checked{0};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_DEMOCLIENTALGS_H
