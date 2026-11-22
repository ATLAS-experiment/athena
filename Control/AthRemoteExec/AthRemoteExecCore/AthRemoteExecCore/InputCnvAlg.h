/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECGATEALG_H
#define ATHREMOTEEXECCORE_REMOTEEXECGATEALG_H

/**
 * @file InputCnvAlg.h
 * @brief The first algorithm of every remote sequence.
 *
 * This is the first algorithm that needs to be execuded for every sequence on
 * the server-side. It retrieves Codec tools, the request body from SG, and does
 * some basic checks. It unpacks binary data send by the client, with Codec
 * tools, which are then written to the SG to be later used by the payload.
 */

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "IPayloadCodec.h"
#include "RequestDescriptor.h"
#include "StoreGate/ReadHandleKey.h"

namespace AthRemoteExec {

class InputCnvAlg : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  Gaudi::Property<std::string> m_sequence{
      this, "SequenceName", "",
      "The name of the sequence that this algorithm handles"};

  SG::ReadHandleKey<RequestDescriptor> m_request{
      this, "Request", "RemoteExecRequest", "The request body"};

  ToolHandleArray<IPayloadCodec> m_decoders{
      this, "Decoders", {},
      "List of Codecs that provide input data to the sequence"};
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_REMOTEEXECGATEALG_H
