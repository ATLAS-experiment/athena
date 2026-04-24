/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../MDT_RawDataProviderTool.h"
#include "../MdtCsmContByteStreamTool.h"
#include "../MdtRDO_Decoder.h"
#include "../MdtROD_Decoder.h"
#include "../MdtRdoToPrepDataToolMT.h"

DECLARE_COMPONENT(Muon::MdtRdoToPrepDataToolMT)
DECLARE_COMPONENT(Muon::MdtCsmContByteStreamTool)
DECLARE_COMPONENT(Muon::MDT_RawDataProviderTool)
DECLARE_COMPONENT(Muon::MdtRDO_Decoder)
DECLARE_COMPONENT(MdtROD_Decoder)
