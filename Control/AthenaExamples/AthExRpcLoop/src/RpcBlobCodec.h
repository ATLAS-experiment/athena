/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCBLOBCODEC_H
#define ATHEXRPCLOOP_RPCBLOBCODEC_H

/**
 * @file RpcBlobCodec.h
 * @brief The @c protobuf encoding: bytes in, bytes out, nothing parsed.
 *
 * The only codec the framework defines. It records a payload's bytes as an
 * AthExRpc::RpcBlob under the boundary's key and reads them back the same way,
 * so the schema is never interpreted here -- the fragment that published it
 * owns the algorithms that convert it (see RpcBlob.h).
 *
 * It links no ROOT and knows no EDM, which is the point: everything in this
 * library can be built and reviewed without a serialisation library or a
 * detector anywhere near it.
 */

#include "RpcPayloadCodec.h"

namespace AthExRpc::PayloadCodec {

/// The process-wide instance, registered under "protobuf".
const IPayloadCodec& blobCodec();

/// The tag it answers to.
extern const char* const protobufEncoding;

}  // namespace AthExRpc::PayloadCodec

#endif  // ATHEXRPCLOOP_RPCBLOBCODEC_H
