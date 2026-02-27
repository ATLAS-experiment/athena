#include "../MuonCacheCreator.h"
#include "MuonByteStream/MuonRawDataProvider.h"
#include "MuonByteStream/CscRdoContByteStreamCnv.h"
#include "MuonByteStream/MdtCsmContByteStreamCnv.h"
#include "MuonByteStream/RpcPadContByteStreamCnv.h"
#include "MuonByteStream/TgcRdoContByteStreamCnv.h"

DECLARE_COMPONENT(Muon::MuonRawDataProvider)
DECLARE_CONVERTER(MdtCsmContByteStreamCnv)
DECLARE_CONVERTER(CscRdoContByteStreamCnv)
DECLARE_CONVERTER(RpcPadContByteStreamCnv)
DECLARE_CONVERTER(TgcRdoContByteStreamCnv)

DECLARE_COMPONENT(MuonCacheCreator)
