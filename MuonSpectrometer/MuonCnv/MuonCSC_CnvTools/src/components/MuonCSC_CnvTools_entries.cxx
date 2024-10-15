/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "../CSC_RawDataProviderToolMT.h"
#include "../CscDigitToCscRDOTool.h"
#include "../CscRDO_Decoder.h"
#include "../CscROD_Decoder.h"
#include "../CscRdoContByteStreamTool.h"
#include "../CscRdoToCscPrepDataToolMT.h"

DECLARE_COMPONENT(Muon::CscRdoToCscPrepDataToolMT)
DECLARE_COMPONENT(Muon::CscRdoContByteStreamTool)
DECLARE_COMPONENT(Muon::CSC_RawDataProviderToolMT)
DECLARE_COMPONENT(Muon::CscROD_Decoder)
DECLARE_COMPONENT(Muon::CscRDO_Decoder)
DECLARE_COMPONENT(CscDigitToCscRDOTool)
