/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECBOUNDARY_H
#define ATHEXREMOTEEXEC_REMOTEEXECBOUNDARY_H

/**
 * @file RemoteExecBoundary.h
 * @brief A boundary as *data*, for the one component that needs it that way.
 *
 * The algorithms do not use this. They are configured with codec tools, which
 * carry the same information as components (see IPayloadCodec.h), and they
 * never parse anything.
 *
 * The loop manager does, and only the loop manager. It is a service: it has no
 * event slot, no store, and no business decoding a payload. What it needs is a
 * *description* of every boundary, so that it can advertise the menu over
 * ListSequences and reject a malformed request before it costs a slot. Hence a
 * string form, produced by the same configuration that builds the tools, so the
 * two cannot drift.
 *
 * Three fields, and all three are the wire contract:
 *
 *   key # encoding # schema
 *
 * Nothing Athena-side is advertised, because nothing outside the job needs it:
 * a codec declares to the scheduler whatever it produces, so no client has to
 * be told a StoreGate type in order to close a graph. What a client of another
 * language could act on is the wire contract, and that is exactly what is here.
 *
 * @c schema may be empty. A codec that carries exactly one payload type is
 * named completely by its encoding tag and has nothing to put there.
 *
 * '#' rather than ':' or ',' because a schema may be a C++ type name, and those
 * contain both ("std::vector<double>", "std::map<int,int>").
 */

#include <string>

namespace AthExRemoteExec {

/// One end of a fragment boundary, as the configuration declared it.
struct Boundary {
  std::string key;
  std::string encoding;
  std::string schema;  ///< may be empty; see above
};

/// Parse "key#encoding#schema".
/// @return false if the spec is malformed; @c boundary is then unspecified.
bool parseBoundary( const std::string& spec, Boundary& boundary );

}  // namespace AthExRemoteExec

#endif  // ATHEXREMOTEEXEC_RPCBOUNDARY_H
