/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECGATEALG_H
#define ATHEXREMOTEEXEC_REMOTEEXECGATEALG_H

/**
 * @file RemoteExecGateAlg.h
 * @brief First algorithm of every fragment: selects it and unpacks its inputs.
 *
 * The scheduler owns whole-event control flow, so a request cannot pick a
 * sub-graph by name the way a serial dispatcher could. The supported mechanism
 * is control-flow gating: every fragment is a seqAND whose first member is one
 * of these. If the request does not name this fragment the gate fails the
 * filter and the rest of the seqAND is skipped.
 *
 * When the fragment *is* selected, the gate decodes the request payload into
 * the fragment's input keys. Two consequences worth being explicit about:
 *
 * - It makes the gate the producer of those keys, so the data-flow graph is
 *   closed and the scheduler can validate the wiring. The gate does not declare
 *   them: each codec declares what it touches, and a codec on a gate is one
 *   that writes. Where the codec has a C++ type to name it does that with an
 *   ordinary handle key; where it has none, because carrying anything is the
 *   point, it declares a key built from the CLID it resolved at initialize().
 *   Either way the statement is made once, by the component that knows.
 * - The genericity stops here. Downstream everything is an ordinary StoreGate
 *   object under an ordinary CLID, read through ordinary typed handles, so
 *   payload algorithms are RemoteExec-ignorant and shareable with a normal job.
 *
 * There is one class, instantiated once per fragment, and it enumerates no
 * types at all: each boundary arrives as a configured codec tool that knows its
 * own key, schema and mechanism. This algorithm knows only the request name
 * that selects it, and which codecs to hand the payloads to.
 */

#include "IPayloadCodec.h"
#include "RemoteExecRequestDescriptor.h"

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>

namespace AthExRemoteExec {

class RemoteExecGateAlg : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute( const EventContext& ctx ) const override;

private:
  Gaudi::Property<std::string> m_sequence{
      this, "SequenceName", "",
      "Name of the fragment this gate guards. A request naming any other "
      "sequence fails the filter"};

  SG::ReadHandleKey<RemoteExecRequestDescriptor> m_request{
      this, "Request", "RemoteExecRequest", "The request, injected by the loop manager"};

  ToolHandleArray<IPayloadCodec> m_inputs{
      this, "Inputs", {},
      "One codec per input boundary. Each carries its own key, schema and "
      "mechanism, so this algorithm parses nothing and knows no encodings"};
};

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_RPCGATEALG_H
