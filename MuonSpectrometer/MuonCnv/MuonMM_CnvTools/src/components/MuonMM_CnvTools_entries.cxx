/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "../MmRdoToPrepDataToolMT.h"
#include "../MM_RDO_Decoder.h"
#include "../MM_ROD_Decoder.h"
#include "../NSWMMTP_ROD_Decoder.h"
#include "../MM_RawDataProviderTool.h"
#include "../NSWMMTP_RawDataProviderToolMT.h"

DECLARE_COMPONENT(Muon::MmRdoToPrepDataToolMT)
DECLARE_COMPONENT(Muon::MM_RDO_Decoder)
DECLARE_COMPONENT(Muon::MM_ROD_Decoder)
DECLARE_COMPONENT(Muon::MM_RawDataProviderTool)
DECLARE_COMPONENT(Muon::NSWMMTP_RawDataProviderToolMT)
DECLARE_COMPONENT(Muon::NSWMMTP_ROD_Decoder)
