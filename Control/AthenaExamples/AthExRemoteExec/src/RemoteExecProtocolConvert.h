/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECWIRECONVERT_H
#define ATHEXREMOTEEXEC_REMOTEEXECWIRECONVERT_H

/**
 * @file RemoteExecProtocolConvert.h
 * @brief Conversions between the generated athremoteexec.v1 messages and the plain
 *        C++ types of RemoteExecProtocol.h.
 *
 * This header includes generated protobuf code and must therefore only ever be
 * included from .cxx files belonging to the wire library (see RemoteExecProtocol.h for
 * why). It is not installed and not visible to the rest of the package.
 *
 * A payload is bytes plus two strings, so there is nothing type-specific to
 * convert here -- which is the point: this file does not grow when a fragment
 * adds a payload type.
 */

#include "athremoteexec.pb.h"

#include "RemoteExecProtocol.h"

namespace AthExRemoteExec {

athremoteexec::v1::Status toProto( Status status );
Status fromProto( athremoteexec::v1::Status status );

void toProto( const Payload& payload, athremoteexec::v1::Payload& out );
void fromProto( const athremoteexec::v1::Payload& in, Payload& payload );

void toProto( const ExecuteRequest& request, athremoteexec::v1::ExecuteRequest& out );
void fromProto( const athremoteexec::v1::ExecuteRequest& in, ExecuteRequest& request );

void toProto( const ExecuteReply& reply, athremoteexec::v1::ExecuteReply& out );
/// Adopts the reply's payload bytes instead of copying them. Prefer this
/// wherever the reply is a temporary, which on the server it always is.
void toProto( ExecuteReply&& reply, athremoteexec::v1::ExecuteReply& out );
void fromProto( const athremoteexec::v1::ExecuteReply& in, ExecuteReply& reply );

void toProto( const SequenceInfo& info, athremoteexec::v1::SequenceInfo& out );

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_RPCWIRECONVERT_H
