/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcBlobCodec.h"

#include "RpcBlob.h"

#include "AthenaKernel/DataBucket.h"
#include "AthenaKernel/DataBucketBase.h"
#include "AthenaKernel/DataObjectSharedPtr.h"
#include "AthenaKernel/IProxyDict.h"
#include "SGTools/DataProxy.h"

#include <memory>

namespace AthExRpc::PayloadCodec {

const char* const protobufEncoding = "protobuf";

namespace {

/// @c protobuf: the bytes are the payload, and nothing here opens them.
class BlobCodec final : public IPayloadCodec {
public:
  virtual const std::string& encoding() const override { return m_encoding; }

  virtual StatusCode clidFor( const Boundary& /*boundary*/,
                              IClassIDSvc& /*clidSvc*/, CLID& clid,
                              std::string& /*error*/ ) const override
  {
    // One type, whatever the schema. A boundary's schema is the fragment's
    // business; as far as StoreGate is concerned every payload on this
    // encoding is the same opaque object, which is exactly why the framework
    // needs no dictionary, no CLID database entry and no generated code per
    // fragment.
    clid = ClassID_traits<RpcBlob>::ID();
    return StatusCode::SUCCESS;
  }

  virtual StatusCode record( IProxyDict& store, IClassIDSvc& /*clidSvc*/,
                             const Resolved& resolved, const Payload& payload,
                             std::string& error ) const override
  {
    SG::DataObjectSharedPtr<DataObject> bucket(
        new SG::DataBucket<RpcBlob>( std::make_unique<RpcBlob>(
            resolved.boundary.schema, payload.data ) ) );
    if ( store.recordObject( std::move( bucket ), resolved.boundary.key, true,
                             false ) == nullptr ) {
      error = "could not record '" + resolved.boundary.key + "'";
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  virtual StatusCode read( IProxyDict& store, const Resolved& resolved,
                           Payload& payload, std::string& error,
                           const std::vector<Resolved>* /*alsoCrossing*/,
                           std::vector<std::string>* /*dangling*/
                           ) const override
  {
    // By CLID rather than through a typed handle, for the same reason record()
    // does not use one: whatever produced the object did so with an ordinary
    // WriteHandle, and only this boundary is generic.
    const Boundary& boundary = resolved.boundary;
    SG::DataProxy* proxy = store.proxy( resolved.clid, boundary.key );
    DataObject* held = proxy != nullptr ? proxy->accessData() : nullptr;
    auto* bucket = dynamic_cast<DataBucketBase*>( held );
    if ( bucket == nullptr ) {
      error = "'" + boundary.key + "' was not produced";
      return StatusCode::FAILURE;
    }
    const auto* blob = static_cast<const RpcBlob*>( bucket->object() );
    if ( blob == nullptr ) {
      error = "'" + boundary.key + "' holds nothing";
      return StatusCode::FAILURE;
    }
    // The adapter that produced it stamped a schema; if it does not match what
    // the boundary declares, the fragment's two halves disagree and the far
    // side would parse the wrong message. Protobuf is permissive enough that
    // this usually succeeds and yields an empty object, so it is checked here
    // rather than discovered downstream as absent data.
    if ( !blob->schema().empty() && blob->schema() != boundary.schema ) {
      error = "'" + boundary.key + "' holds a '" + blob->schema() +
              "' but this boundary declares '" + boundary.schema + "'";
      return StatusCode::FAILURE;
    }

    payload.key = boundary.key;
    payload.encoding = boundary.encoding;
    payload.schema = boundary.schema;
    payload.data = blob->bytes();
    return StatusCode::SUCCESS;
  }

private:
  const std::string m_encoding = protobufEncoding;
};

}  // anonymous namespace

const IPayloadCodec& blobCodec()
{
  static const BlobCodec instance;
  return instance;
}

}  // namespace AthExRpc::PayloadCodec
