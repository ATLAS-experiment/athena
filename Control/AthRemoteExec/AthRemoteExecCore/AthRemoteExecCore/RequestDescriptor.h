/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECREQUESTDESCRIPTOR_H
#define ATHREMOTEEXECCORE_REMOTEEXECREQUESTDESCRIPTOR_H

/**
 * @file RequestDescriptor.h
 * @brief The request, read by @c InputCnvAlg from SG.
 *
 * Similar to @c ExecuteRequest, but without EventId. Used to pass all needed
 * request details to Gate algorithms.
 */

#include <AthRemoteExecWire/Protocol.h>
#include <AthenaKernel/CLASS_DEF.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace AthRemoteExec {

class RequestDescriptor {
 public:
  RequestDescriptor() = default;
  RequestDescriptor(std::string sequence, uint64_t requestId,
                              std::vector<Payload> inputs)
      : m_sequence(std::move(sequence)),
        m_inputs(std::move(inputs)),
        m_requestId(requestId) {}

  const std::string& sequence() const { return m_sequence; }

  uint64_t requestId() const { return m_requestId; }

  const std::vector<Payload>& inputs() const { return m_inputs; }

  /// Find payload that corresponds to given @c key.
  const Payload* find(const std::string& key) const {
    const auto found = std::find_if(
        m_inputs.begin(), m_inputs.end(),
        [&key](const Payload& payload) { return payload.key == key; });
    return found == m_inputs.end() ? nullptr : &*found;
  }

 private:
  std::string m_sequence;
  std::vector<Payload> m_inputs;
  uint64_t m_requestId = 0;
};

}  // namespace AthRemoteExec

CLASS_DEF(AthRemoteExec::RequestDescriptor, 213558421, 1)

#endif  // ATHREMOTEEXECCORE_REMOTEEXECREQUESTDESCRIPTOR_H
