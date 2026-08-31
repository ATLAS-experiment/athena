/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcPayloadCodec.h"

#include "RpcBlobCodec.h"

#include <algorithm>
#include <map>
#include <mutex>

namespace AthExRpc::PayloadCodec {

namespace {

/// The registry, and the lock that covers it.
///
/// Registration happens as libraries load and lookup happens at initialize(),
/// so contention is nil -- but the two are not otherwise ordered, and a
/// dynamically loaded component library registering while another is resolving
/// is exactly the situation Gaudi's component loading creates. The per-request
/// path never comes here: Resolved caches the pointer.
struct Registry {
  std::mutex mutex;
  std::map<std::string, const IPayloadCodec*> codecs;
};

Registry& registry()
{
  static Registry instance;
  // The one encoding the framework defines. Registered here rather than by a
  // static initialiser in RpcBlobCodec.cxx so that its ordering against this
  // map's construction is not a question anyone has to answer.
  static const bool builtIn = [] {
    instance.codecs.emplace( blobCodec().encoding(), &blobCodec() );
    return true;
  }();
  (void)builtIn;
  return instance;
}

}  // anonymous namespace

bool parseBoundary( const std::string& spec, Boundary& boundary )
{
  const size_t first = spec.find( '#' );
  if ( first == std::string::npos ) {
    return false;
  }
  const size_t second = spec.find( '#', first + 1 );
  if ( second == std::string::npos ) {
    // "key#schema" was once legal, with the encoding inferred from the type.
    // It is not any more; see RpcPayloadCodec.h.
    return false;
  }
  boundary.key = spec.substr( 0, first );
  boundary.encoding = spec.substr( first + 1, second - first - 1 );
  boundary.schema = spec.substr( second + 1 );
  return !boundary.key.empty() && !boundary.encoding.empty() &&
         !boundary.schema.empty() &&
         boundary.schema.find( '#' ) == std::string::npos;
}

StatusCode registerCodec( const IPayloadCodec& codec )
{
  Registry& reg = registry();
  const std::lock_guard<std::mutex> lock( reg.mutex );
  return reg.codecs.emplace( codec.encoding(), &codec ).second
             ? StatusCode::SUCCESS
             : StatusCode::FAILURE;
}

const IPayloadCodec* findCodec( const std::string& encoding )
{
  Registry& reg = registry();
  const std::lock_guard<std::mutex> lock( reg.mutex );
  const auto found = reg.codecs.find( encoding );
  return found == reg.codecs.end() ? nullptr : found->second;
}

std::vector<std::string> knownEncodings()
{
  Registry& reg = registry();
  const std::lock_guard<std::mutex> lock( reg.mutex );
  std::vector<std::string> tags;
  tags.reserve( reg.codecs.size() );
  for ( const auto& [tag, codec] : reg.codecs ) {
    tags.push_back( tag );
  }
  return tags;
}

}  // namespace AthExRpc::PayloadCodec
