/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHEXREMOTEEXEC_REMOTEEXECREPLYSTAGING_H
#define ATHEXREMOTEEXEC_REMOTEEXECREPLYSTAGING_H

/**
 * @file RemoteExecReplyStaging.h
 * @brief The reply, staged in the slot store for the loop manager to collect.
 *
 * The counterpart of RemoteExecRequestDescriptor on the way out. Each fragment's pack
 * algorithm records exactly one of these under its own key; the loop manager
 * reads it after the scheduler reports the event finished, and only then clears
 * the slot. That is why this manager must not subscribe to the EndAlgorithms
 * incident the way AthenaHiveEventLoopMgr does -- the store has to outlive the
 * event.
 */

#include "RemoteExecProtocol.h"

#include "AthenaKernel/CLASS_DEF.h"

#include <utility>

namespace AthExRemoteExec {

class RemoteExecReplyStaging {
public:
  RemoteExecReplyStaging() = default;
  explicit RemoteExecReplyStaging( ExecuteReply reply ) : m_reply( std::move( reply ) ) {}

  const ExecuteReply& reply() const { return m_reply; }

  /// Hand the reply over, leaving this object empty.
  ///
  /// The loop manager is the only reader and it clears the slot immediately
  /// afterwards, so there is nothing left to observe the emptied state -- and
  /// copying instead duplicated the entire payload for the third time on the
  /// way out.
  ExecuteReply takeReply() { return std::move( m_reply ); }

private:
  ExecuteReply m_reply;
};

}  // namespace AthExRemoteExec

CLASS_DEF( AthExRemoteExec::RemoteExecReplyStaging, 213558422, 1 )

#endif  // ATHEXREMOTEEXEC_RPCREPLYSTAGING_H
