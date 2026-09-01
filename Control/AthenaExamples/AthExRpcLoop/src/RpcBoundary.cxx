/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcBoundary.h"

#include <vector>

namespace AthExRpc {

bool parseBoundary( const std::string& spec, Boundary& boundary )
{
  std::vector<std::string> fields;
  size_t start = 0;
  for ( size_t hash = spec.find( '#' ); ; hash = spec.find( '#', start ) ) {
    if ( hash == std::string::npos ) {
      fields.push_back( spec.substr( start ) );
      break;
    }
    fields.push_back( spec.substr( start, hash - start ) );
    start = hash + 1;
    if ( fields.size() > 3 ) {
      return false;  // more than three fields: not a boundary
    }
  }
  if ( fields.size() != 3 ) {
    // The encoding is required rather than inferred from the type: a wire
    // format is not a property of a C++ type, and inferring it would make a
    // boundary's serialisation change silently when somebody edits a handle.
    // See IPayloadCodec.h.
    return false;
  }
  boundary.key = fields[0];
  boundary.encoding = fields[1];
  boundary.schema = fields[2];
  // The schema may legitimately be empty; the other two may not.
  return !boundary.key.empty() && !boundary.encoding.empty();
}

}  // namespace AthExRpc
