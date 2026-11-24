/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ATHREMOTEEXECCODECS_BYTESTREAMCODEC_H
#define ATHREMOTEEXECCODECS_BYTESTREAMCODEC_H

#include <AthRemoteExecCodecs/RawEventCache.h>
#include <AthRemoteExecCore/PayloadCodecBase.h>
#include <ByteStreamCnvSvcBase/IROBDataProviderSvc.h>
#include <StoreGate/WriteHandleKey.h>

#include <string>
#include <vector>

namespace AthRemoteExec {

/// @brief Encode and decode RawEvent data.
///
/// Codec for encoding/decoding RawEvent data into/from plain bytes. It is meant to only transfer data from client
/// (reading RAW file/bytestream) to server. On the client-side RawEvent is read directly from ROBDataProviderSvc, while
/// on the server-side the decoded data is stored in StoreGate as RawEventCache. Server-side ROBDataProviderSvc is then
/// notified, via setNextEvent() call, that RawEvent stored in StoreGate is the next one.
class BytestreamCodec final : public PayloadCodecBase {
public:
  using PayloadCodecBase::PayloadCodecBase;

  virtual const std::string& encoding() const override { return s_encoding; }

protected:
  virtual StatusCode initializeBoundary() override;
  virtual StatusCode doDecode(const EventContext& ctx, IProxyDict& store, const Payload& payload) const override;
  virtual StatusCode doEncode(const EventContext& ctx, IProxyDict& store, Payload& payload,
                              const std::vector<const IPayloadCodec*>& alsoCrossing,
                              std::vector<std::string>& dangling) const override;

private:
  /// Writing to SG only on the receiving side
  SG::WriteHandleKey<RawEventCache> m_decoded{this, "Decoded", "", "Key for RawEvent cache object"};

  /// Read RawEvent directly on the sending side or set the next event on the receiving side
  ServiceHandle<IROBDataProviderSvc> m_robsSvc{this, "ROBDataProvider", "ROBDataProviderSvc/ROBDataProviderSvc"};

  static const std::string s_encoding;
};

}  // namespace AthRemoteExec

#endif  // ATHREMOTEEXECCODECS_BYTESTREAMCODEC_H
