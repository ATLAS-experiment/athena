/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "../CscRdoToCscDigit.h"
#include "../MM_DigitToRDO.h"
#include "../MM_RdoToDigit.h"
#include "../MdtRdoToMdtDigit.h"
#include "../RpcRdoToRpcDigit.h"
#include "../STGC_DigitToRDO.h"
#include "../STGC_RdoToDigit.h"
#include "../TgcRdoToTgcDigit.h"
#include "../CscDigitToCscRDO.h"
#include "../MdtDigitToMdtRDO.h"
#include "../RpcDigitToRpcRDO.h"
#include "../NrpcDigitToNrpcRDO.h"
#include "../TgcDigitToTgcRDO.h"


DECLARE_COMPONENT(Muon::NrpcDigitToNrpcRDO)
DECLARE_COMPONENT(Muon::RpcRdoToRpcDigit)

DECLARE_COMPONENT(RpcDigitToRpcRDO)
DECLARE_COMPONENT(MdtDigitToMdtRDO)
DECLARE_COMPONENT(TgcDigitToTgcRDO)
DECLARE_COMPONENT(CscDigitToCscRDO)
DECLARE_COMPONENT(STGC_DigitToRDO)
DECLARE_COMPONENT(MM_DigitToRDO)

DECLARE_COMPONENT(CscRdoToCscDigit)
DECLARE_COMPONENT(MdtRdoToMdtDigit)
DECLARE_COMPONENT(TgcRdoToTgcDigit)
DECLARE_COMPONENT(STGC_RdoToDigit)
DECLARE_COMPONENT(MM_RdoToDigit)
