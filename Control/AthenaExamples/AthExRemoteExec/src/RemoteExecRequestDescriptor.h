/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECREQUESTDESCRIPTOR_H
#define ATHEXREMOTEEXEC_REMOTEEXECREQUESTDESCRIPTOR_H

/**
 * @file RemoteExecRequestDescriptor.h
 * @brief The request as seen from inside the event.
 *
 * The loop manager cannot unpack a request itself: under the scheduler the
 * graph is executed as a whole, and the manager only gets to prepare the store
 * before handing the event over. So the request travels into the event as an
 * ordinary StoreGate object, and the gate algorithm of each fragment reads it
 * to decide whether the request is for it.
 */

#include "RemoteExecProtocol.h"

#include "AthenaKernel/CLASS_DEF.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace AthExRemoteExec {

class RemoteExecRequestDescriptor {
public:
  RemoteExecRequestDescriptor() = default;
  RemoteExecRequestDescriptor( std::string sequence, uint64_t requestId,
                        std::vector<Payload> inputs )
      : m_sequence( std::move( sequence ) ),
        m_inputs( std::move( inputs ) ),
        m_requestId( requestId )
  {
  }

  /// Name of the fragment the client asked for.
  const std::string& sequence() const { return m_sequence; }

  /// Client correlation id, echoed back in the reply.
  uint64_t requestId() const { return m_requestId; }

  const std::vector<Payload>& inputs() const { return m_inputs; }

  /// The payload supplied under @c key, or nullptr if the request has none.
  const Payload* find( const std::string& key ) const
  {
    const auto found = std::find_if(
        m_inputs.begin(), m_inputs.end(),
        [&key]( const Payload& payload ) { return payload.key == key; } );
    return found == m_inputs.end() ? nullptr : &*found;
  }

private:
  std::string m_sequence;
  std::vector<Payload> m_inputs;
  uint64_t m_requestId = 0;
};

}  // namespace AthExRemoteExec

CLASS_DEF( AthExRemoteExec::RemoteExecRequestDescriptor, 213558421, 1 )

#endif  // ATHEXREMOTEEXEC_RPCREQUESTDESCRIPTOR_H
