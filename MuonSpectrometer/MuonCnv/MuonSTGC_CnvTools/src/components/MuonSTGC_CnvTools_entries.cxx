/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "../sTgcRdoToPrepDataToolMT.h"
#include "../STGC_RawDataProviderTool.h"
#include "../STGC_RawDataProviderTool.h"
#include "../STGC_ROD_Decoder.h"
#include "../STGC_RDO_Decoder.h"
#include "../PadTrig_ROD_Decoder.h"
#include "../PadTrig_RawDataProviderTool.h"
#include "../PadTrig_RawDataProviderTool.h"

#include "../NSWTP_RawDataProviderTool.h"
#include "../NSWTP_ROD_Decoder.h"


DECLARE_COMPONENT( Muon::sTgcRdoToPrepDataToolMT )
DECLARE_COMPONENT( Muon::STGC_RawDataProviderTool )

DECLARE_COMPONENT( Muon::STGC_ROD_Decoder )
DECLARE_COMPONENT( Muon::STGC_RDO_Decoder )
DECLARE_COMPONENT( Muon::PadTrig_ROD_Decoder )
DECLARE_COMPONENT( Muon::PadTrig_RawDataProviderTool )
DECLARE_COMPONENT( Muon::NSWTP_RawDataProviderTool)
DECLARE_COMPONENT( Muon::NSWTP_ROD_Decoder)
