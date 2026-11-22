/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_IPAYLOADCODEC_H
#define ATHREMOTEEXECCORE_IPAYLOADCODEC_H

/**
 * @file IPayloadCodec.h
 * @brief An @c AthAlgTool configured with the boundary (see BoundaryInfo) it
 * serves.
 *
 * Codec declares what StoreGate object it writes, what is received as Payload,
 * and reads, what is later send as Payload. Implementations are retrieved once
 * and then called from every slot concurrently, so every method here must be
 * thread-safe and const.
 */

#include <AthRemoteExecWire/Protocol.h>

#include <string>
#include <vector>

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"

namespace AthRemoteExec {

class IPayloadCodec : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IPayloadCodec, 1, 0);

  /// The RPC protocol this codec answers to. Fixed by the class.
  virtual const std::string& encoding() const = 0;

  /// The name of the boundary/SG key this instance serves.
  virtual const std::string& key() const = 0;

  /// What the bytes are. May be empty.
  virtual const std::string& schema() const = 0;

  /// Decode @c payload into the event's store.
  virtual StatusCode decode(const EventContext& ctx,
                            const Payload& payload) const = 0;

  /**
   * @brief Encode @c payload from the event's store
   *
   * @param alsoCrossing Other codecs that reference this codec's payload in
   * some way, e.g. links in xAOD containers
   * @param dangling List of related keys that are not carried by this codec or
   * codes in alsoCrossing
   */
  virtual StatusCode encode(
      const EventContext& ctx, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling) const = 0;
};

/// Message that joins what's returned by dangling in IPayloadCodec::encode()
std::string describeDangling(const std::vector<std::string>& dangling);

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_IPAYLOADCODEC_H
