/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECPACKALG_H
#define ATHREMOTEEXECCORE_REMOTEEXECPACKALG_H

/**
 * @file OutputCnvAlg.h
 * @brief The last algorithm of every remote sequence.
 *
 * This is the last algorithm that needs to be execuded for every sequence on
 * the server-side. It retrieves Codec tools, the request body from SG, and does
 * some basic checks. It packs binary data to be later sent to the client, with
 * Codec tools.
 */

#include <atomic>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "IPayloadCodec.h"
#include "ReplyStaging.h"
#include "RequestDescriptor.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

namespace AthRemoteExec {

class OutputCnvAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  SG::ReadHandleKey<RequestDescriptor> m_request{
      this, "Request", "RemoteExecRequest",
      "The request, only for the request id to echo back"};
  SG::WriteHandleKey<ReplyStaging> m_reply{this, "Reply", "",
                                                     "Prepared reply"};

  ToolHandleArray<IPayloadCodec> m_encoders{
      this, "Encoders", {}, "List of Codecs that pack output data of the sequence"};

  /// References pointing outside the payload set are a configuration fact, so
  /// they are reported on the first reply that has any and not again.
  mutable std::atomic<bool> m_warnedDangling{false};

  /// The same codecs as raw pointers, built once, because that is what
  /// IPayloadCodec::read wants for its cross-reference check and rebuilding it
  /// per request would allocate on every reply.
  std::vector<const IPayloadCodec*> m_crossing;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_REMOTEEXECPACKALG_H
