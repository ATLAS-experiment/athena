/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCORE_REMOTEEXECPAYLOADCODECBASE_H
#define ATHREMOTEEXECCORE_REMOTEEXECPAYLOADCODECBASE_H

/**
 * @file PayloadCodecBase.h
 * @brief The base class of all Coded tools.
 *
 * The class from which all Codec tools need to derive. It takes care of
 * declaring all relevant properties and implementing methods. Derived tool
 * needs to implement @c doRead and @c doRecord that contain encoding/decoding
 * logic. Derived tool is also reponsible for checking whether the data are
 * intended for it.
 */

#include <AthenaBaseComps/AthAlgTool.h>
#include <Gaudi/Property.h>

#include <string>
#include <vector>

#include "IPayloadCodec.h"

namespace AthRemoteExec {

class PayloadCodecBase : public extends<AthAlgTool, IPayloadCodec> {
 public:
  using extends::extends;

  virtual StatusCode initialize() override;

  virtual const std::string& key() const override final { return m_key; }
  virtual const std::string& schema() const override final { return m_schema; }

  virtual StatusCode decode(const EventContext& ctx,
                            const Payload& payload) const override final;
  virtual StatusCode encode(
      const EventContext& ctx, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling) const override final;

 protected:
  /// Codec direction
  bool writingToSG() const { return m_writing; }

  /// Called from ::initialize()
  virtual StatusCode initializeBoundary() { return StatusCode::SUCCESS; }

  /// Decode @c payload.data and record it onto a SG
  virtual StatusCode doDecode(const EventContext& ctx, IProxyDict& store,
                              const Payload& payload) const = 0;

  /// Read input data (SG, RawEvent, etc.) and encode it into a @c payload.data
  virtual StatusCode doEncode(
      const EventContext& ctx, IProxyDict& store, Payload& payload,
      const std::vector<const IPayloadCodec*>& alsoCrossing,
      std::vector<std::string>& dangling) const = 0;

 private:
  Gaudi::Property<std::string> m_key{this, "Key", "", "see BoundaryInfo::key"};
  Gaudi::Property<std::string> m_schema{this, "Schema", "",
                                        "see BoundaryInfo::schema"};
  Gaudi::Property<std::string> m_direction{
      this, "Direction", "",
      "Direction in which this Codec operates: 'ReadFromSG' -> encoding, "
      "'WriteFromSG' -> decoding"};

  /// The store belonging to @c ctx. Errors and returns null if there is none.
  IProxyDict* storeOf(const EventContext& ctx) const;

  bool m_writing = false;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCORE_REMOTEEXECPAYLOADCODECBASE_H
