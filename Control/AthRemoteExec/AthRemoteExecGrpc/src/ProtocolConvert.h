/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECGRPC_REMOTEEXECWIRECONVERT_H
#define ATHREMOTEEXECGRPC_REMOTEEXECWIRECONVERT_H

/**
 * @file ProtocolConvert.h
 * @brief Conversions between the generated gRPC messages and the plain C++
 * types from Protocol.h.
 */

#include <AthRemoteExecWire/Protocol.h>

#include "athremoteexec.pb.h"

namespace AthRemoteExec {

athremoteexec::v1::Status toProto(Status status);
Status fromProto(athremoteexec::v1::Status status);

void toProto(const Payload& payload, athremoteexec::v1::Payload& out);
void fromProto(const athremoteexec::v1::Payload& in, Payload& payload);

void toProto(const ExecuteRequest& request,
             athremoteexec::v1::ExecuteRequest& out);
void fromProto(const athremoteexec::v1::ExecuteRequest& in,
               ExecuteRequest& request);

void toProto(const ExecuteReply& reply, athremoteexec::v1::ExecuteReply& out);
void toProto(ExecuteReply&& reply, athremoteexec::v1::ExecuteReply& out);
void fromProto(const athremoteexec::v1::ExecuteReply& in, ExecuteReply& reply);

void toProto(const SequenceInfo& info, athremoteexec::v1::SequenceInfo& out);

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXEC_RPCWIRECONVERT_H
