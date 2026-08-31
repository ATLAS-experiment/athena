/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_RPCBLOB_H
#define ATHEXRPCLOOP_RPCBLOB_H

/**
 * @file RpcBlob.h
 * @brief A payload's bytes, in the event store, still unopened.
 *
 * This is where a @c protobuf boundary lands. The gate records one of these
 * under the boundary's key; a fragment-owned adapter algorithm reads it with an
 * ordinary ReadHandle, parses it against the schema it published, and records
 * ordinary typed objects for the rest of the fragment to consume. On the way
 * out the same thing happens in reverse.
 *
 * The framework therefore never parses a fragment's message and never links its
 * generated code -- the reason this class exists rather than the gate handing
 * bytes straight to a decoder. It carries the schema name alongside the bytes
 * so that the adapter can refuse a payload built against a schema it does not
 * implement, instead of parsing one message as another: protobuf's wire format
 * is permissive enough that the wrong message often parses, quietly, into
 * something empty.
 */

#include "AthenaKernel/CLASS_DEF.h"

#include <string>
#include <utility>

namespace AthExRpc {

class RpcBlob {
public:
  RpcBlob() = default;
  RpcBlob( std::string schema, std::string bytes )
      : m_schema( std::move( schema ) ), m_bytes( std::move( bytes ) )
  {
  }

  /// Fully qualified name of the message these bytes are, as declared at the
  /// boundary and carried on the wire.
  const std::string& schema() const { return m_schema; }

  /// The bytes themselves. Binary, and may contain embedded nulls.
  const std::string& bytes() const { return m_bytes; }

private:
  std::string m_schema;
  std::string m_bytes;
};

}  // namespace AthExRpc

CLASS_DEF( AthExRpc::RpcBlob, 213558425, 1 )

#endif  // ATHEXRPCLOOP_RPCBLOB_H
