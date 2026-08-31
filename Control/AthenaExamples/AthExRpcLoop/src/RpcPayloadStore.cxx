/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcPayloadStore.h"

#include <utility>

namespace AthExRpc::PayloadStore {

namespace {

std::string listEncodings()
{
  std::string list;
  for ( const std::string& tag : PayloadCodec::knownEncodings() ) {
    list += ( list.empty() ? "" : ", " ) + tag;
  }
  return list.empty() ? "none" : list;
}

}  // anonymous namespace

StatusCode resolve( const std::vector<std::string>& specs,
                    IClassIDSvc& clidSvc, std::vector<Resolved>& resolved,
                    std::string& error )
{
  for ( const std::string& spec : specs ) {
    Resolved entry;
    if ( !PayloadCodec::parseBoundary( spec, entry.boundary ) ) {
      error = "'" + spec + "' is not of the form key#encoding#schema";
      return StatusCode::FAILURE;
    }
    entry.codec = PayloadCodec::findCodec( entry.boundary.encoding );
    if ( entry.codec == nullptr ) {
      error = "'" + spec + "' uses encoding '" + entry.boundary.encoding +
              "', which no codec in this job implements (have: " +
              listEncodings() + ")";
      return StatusCode::FAILURE;
    }
    if ( entry.codec->clidFor( entry.boundary, clidSvc, entry.clid, error )
             .isFailure() ) {
      error = "'" + spec + "': " + error;
      return StatusCode::FAILURE;
    }
    resolved.push_back( std::move( entry ) );
  }
  return StatusCode::SUCCESS;
}

StatusCode record( IProxyDict& store, IClassIDSvc& clidSvc,
                   const Resolved& resolved, const Payload& payload,
                   std::string& error )
{
  const PayloadCodec::Boundary& boundary = resolved.boundary;
  // Checked here rather than in each codec: it is the same check whatever the
  // encoding, and getting it wrong is the difference between "this is not what
  // you declared" and a codec confidently misreading someone else's bytes.
  if ( payload.encoding != boundary.encoding ) {
    error = "'" + boundary.key + "' arrived with encoding '" +
            payload.encoding + "' but this fragment declares '" +
            boundary.encoding + "'";
    return StatusCode::FAILURE;
  }
  if ( payload.schema != boundary.schema ) {
    error = "'" + boundary.key + "' arrived as '" + payload.schema +
            "' but this fragment declares '" + boundary.schema + "'";
    return StatusCode::FAILURE;
  }
  return resolved.codec->record( store, clidSvc, resolved, payload, error );
}

StatusCode read( IProxyDict& store, const Resolved& resolved, Payload& payload,
                 std::string& error, const std::vector<Resolved>* alsoCrossing,
                 std::vector<std::string>* dangling )
{
  return resolved.codec->read( store, resolved, payload, error, alsoCrossing,
                               dangling );
}

std::string describeDangling( const std::vector<std::string>& dangling )
{
  std::string list;
  for ( const std::string& entry : dangling ) {
    list += ( list.empty() ? "" : ", " ) + entry;
  }
  return list;
}

}  // namespace AthExRpc::PayloadStore
