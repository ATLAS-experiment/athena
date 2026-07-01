/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "../TGC_RawDataProviderTool.h"
#include "../TGC_RodDecoderRawdata.h"
#include "../TGC_RodDecoderReadout.h"
#include "../TgcRDO_Decoder.h"
#include "../TgcRdoContByteStreamTool.h"
#include "../TgcRdoToPrepDataToolMT.h"

DECLARE_COMPONENT(Muon::TgcRdoToPrepDataToolMT)
DECLARE_COMPONENT(Muon::TgcRdoContByteStreamTool)
DECLARE_COMPONENT(Muon::TGC_RodDecoderReadout)
DECLARE_COMPONENT(Muon::TGC_RodDecoderRawdata)
DECLARE_COMPONENT(Muon::TGC_RawDataProviderTool)
DECLARE_COMPONENT(Muon::TgcRDO_Decoder)
