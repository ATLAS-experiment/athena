/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECREQUESTALG_H
#define ATHREMOTEEXECCORE_REMOTEEXECREQUESTALG_H

/**
 * @file RequestAlg.h
 * @brief Calls a remote sequence from an ordinary Athena job.
 *
 * The client-side version of both @c InputCnvAlg and @c
 * OutputCnvAlg. It reads the declared inputs from SG, sends them, waits,
 * and records what comes back under the declared keys.
 */

#include <AthRemoteExecWire/Client.h>
#include <AthenaBaseComps/AthAsynchronousAlgorithm.h>
#include <CxxUtils/checker_macros.h>
#include <Gaudi/Property.h>
#include <GaudiKernel/ToolHandle.h>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "IPayloadCodec.h"

namespace AthRemoteExec {

class RequestAlg : public AthAsynchronousAlgorithm {
 public:
  using AthAsynchronousAlgorithm::AthAsynchronousAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  Gaudi::Property<std::string> m_target{
      this, "Target", "",
      "Server as 'host:port', or the path of a file the server writes its port"
      "into"};

  Gaudi::Property<double> m_readyTimeout{
      this, "ReadyTimeout", 120.0,
      "Seconds to wait in initialize() for the server to answer"};

  Gaudi::Property<std::string> m_sequence{
      this, "SequenceName", "",
      "The name of the sequence that this algorithm handles"};

  ToolHandleArray<IPayloadCodec> m_inputs{
      this, "Decoders", {},
      "List of Codecs that pack input data for the remote sequence"};

  ToolHandleArray<IPayloadCodec> m_outputs{
      this, "Encoders", {},
      "List of Codecs that unpack output data from the remote sequence"};

  /// References pointing outside the payload set are a configuration fact, so
  /// they are reported on the first request that has any and not again.
  mutable std::atomic<bool> m_warnedDangling{false};

  /// The input codecs as raw pointers; see OutputCnvAlg for why.
  std::vector<const IPayloadCodec*> m_crossing;

  /// Server pointer, shared between slots.
  mutable std::unique_ptr<Client> m_client ATLAS_THREAD_SAFE;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_RPCREQUESTALG_H
