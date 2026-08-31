/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXRPCLOOP_DEMOSCHEMA_H
#define ATHEXRPCLOOP_DEMOSCHEMA_H

/**
 * @file DemoSchema.h
 * @brief The demonstration fragment's payload conversions, in plain C++.
 *
 * proto/athexrpc_demo.proto is this fragment's schema and DemoSchema.cxx is the
 * only file that has ever seen its generated code. Everything else in src/demo/
 * -- the four adapter algorithms -- goes through this header, exactly as the
 * rest of the package goes through RpcWire.h, and for the same reason (see
 * RpcWire.h for the protobuf-runtime argument).
 *
 * Nothing outside src/demo/ includes this file. That is the property the
 * demonstration exists to show: a fragment publishes a schema and ships the
 * algorithms that convert it, and the transport, the loop manager, the gate and
 * the pack algorithm are unchanged by its existence.
 */

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace AthExRpc::DemoSchema {

/// @name Schema names, as they appear in Payload::schema
/// @{
extern const char* const ints;     ///< athexrpc.demo.v1.Ints
extern const char* const doubles;  ///< athexrpc.demo.v1.Doubles
/// @}

/// One entry of an Ints message: a StoreGate key and its value.
struct NamedInt {
  std::string name;
  int64_t value = 0;
};

/// @return false on failure, with @c error set.
bool encodeInts( const std::vector<NamedInt>& values, std::string& bytes,
                 std::string& error );
bool decodeInts( std::string_view bytes, std::vector<NamedInt>& values,
                 std::string& error );

bool encodeDoubles( const std::string& name, const std::vector<double>& values,
                    std::string& bytes, std::string& error );
bool decodeDoubles( std::string_view bytes, std::string& name,
                    std::vector<double>& values, std::string& error );

}  // namespace AthExRpc::DemoSchema

#endif  // ATHEXRPCLOOP_DEMOSCHEMA_H
