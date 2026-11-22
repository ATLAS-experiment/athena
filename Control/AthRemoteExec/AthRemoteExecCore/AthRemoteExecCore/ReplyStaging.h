/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECREPLYSTAGING_H
#define ATHREMOTEEXECCORE_REMOTEEXECREPLYSTAGING_H

/**
 * @file ReplyStaging.h
 * @brief The reply, written by @c OutputCnvAlg onto SG.
 *
 * Holds onto @c ExecuteReply, which is later moved from this object in @c
 * RemoteExecHiveEventLoopMgr::buildReply(), to be later send back to the
 * client.
 */

#include <AthRemoteExecWire/Protocol.h>
#include <AthenaKernel/CLASS_DEF.h>

#include <utility>

namespace AthRemoteExec {

class ReplyStaging {
 public:
  ReplyStaging() = default;
  explicit ReplyStaging(ExecuteReply reply)
      : m_reply(std::move(reply)) {}

  const ExecuteReply& reply() const { return m_reply; }

  /// Hand the reply over, leaving this object empty. The loop manager is the
  /// only reader and it clears the slot immediately afterwards, so there is
  /// nothing left to observe the emptied state.
  ExecuteReply takeReply() { return std::move(m_reply); }

 private:
  ExecuteReply m_reply;
};

}  // namespace AthRemoteExec

CLASS_DEF(AthRemoteExec::ReplyStaging, 213558422, 1)

#endif  // ATHREMOTEEXECCORE_REMOTEEXECREPLYSTAGING_H
