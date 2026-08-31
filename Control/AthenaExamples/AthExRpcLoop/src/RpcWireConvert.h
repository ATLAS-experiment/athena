/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCWIRECONVERT_H
#define ATHEXRPCLOOP_RPCWIRECONVERT_H

/**
 * @file RpcWireConvert.h
 * @brief Conversions between the generated athrpc.v1 messages and the plain
 *        C++ types of RpcWire.h.
 *
 * This header includes generated protobuf code and must therefore only ever be
 * included from .cxx files belonging to the wire library (see RpcWire.h for
 * why). It is not installed and not visible to the rest of the package.
 *
 * A payload is bytes plus two strings, so there is nothing type-specific to
 * convert here -- which is the point: this file does not grow when a fragment
 * adds a payload type.
 */

#include "athrpc.pb.h"

#include "RpcWire.h"

namespace AthExRpc {

athrpc::v1::Status toProto( Status status );
Status fromProto( athrpc::v1::Status status );

void toProto( const Payload& payload, athrpc::v1::Payload& out );
void fromProto( const athrpc::v1::Payload& in, Payload& payload );

void toProto( const ExecuteRequest& request, athrpc::v1::ExecuteRequest& out );
void fromProto( const athrpc::v1::ExecuteRequest& in, ExecuteRequest& request );

void toProto( const ExecuteReply& reply, athrpc::v1::ExecuteReply& out );
/// Adopts the reply's payload bytes instead of copying them. Prefer this
/// wherever the reply is a temporary, which on the server it always is.
void toProto( ExecuteReply&& reply, athrpc::v1::ExecuteReply& out );
void fromProto( const athrpc::v1::ExecuteReply& in, ExecuteReply& reply );

void toProto( const SequenceInfo& info, athrpc::v1::SequenceInfo& out );

}  // namespace AthExRpc

#endif  // ATHEXRPCLOOP_RPCWIRECONVERT_H
