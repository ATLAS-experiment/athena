/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondAlg/CscCondDbAlg.h"
#include "MuonCondAlg/MdtCalibDbAlg.h"
#include "MuonCondAlg/MdtCondDbAlg.h"
#include "MuonCondAlg/MuonAlignmentCondAlg.h"
#include "MuonCondAlg/MuonAlignmentErrorDbAlg.h"
#include "MuonCondAlg/RpcCondDbAlg.h"
#include "MuonCondAlg/TgcCondDbAlg.h"
#include "MuonCondAlg/TgcDigitASDposCondAlg.h"
#include "MuonCondAlg/TgcDigitTimeOffsetCondAlg.h"
#include "MuonCondAlg/TgcDigitCrosstalkCondAlg.h"
#include "MuonCondAlg/TgcDigitEnergyThreshCondAlg.h"
#include "MuonCondAlg/TgcDigitJitterCondAlg.h"
#include "MuonCondAlg/NswCalibDbAlg.h"
#include "MuonCondAlg/NswDcsDbAlg.h"
#include "MuonCondAlg/NswPassivationDbAlg.h"
#include "MuonCondAlg/NswUncertDbAlg.h"
#include "MuonCondAlg/NswAsBuiltCondAlg.h"
#include "MuonCondAlg/MdtAsBuiltCondAlg.h"
#include "MuonCondAlg/CscILinesCondAlg.h"
#include "MuonCondAlg/sTGCAsBuiltCondAlg2.h"
#include "MuonCondAlg/MmCTPCondDbAlg.h"


DECLARE_COMPONENT(CscCondDbAlg)
DECLARE_COMPONENT(MdtCondDbAlg)
DECLARE_COMPONENT(RpcCondDbAlg)
DECLARE_COMPONENT(TgcCondDbAlg)
DECLARE_COMPONENT(MuonAlignmentErrorDbAlg)
DECLARE_COMPONENT(MuonAlignmentCondAlg)
DECLARE_COMPONENT(MdtCalibDbAlg)
DECLARE_COMPONENT(TgcDigitASDposCondAlg)
DECLARE_COMPONENT(TgcDigitTimeOffsetCondAlg)
DECLARE_COMPONENT(TgcDigitCrosstalkCondAlg)
DECLARE_COMPONENT(TgcDigitEnergyThreshCondAlg)
DECLARE_COMPONENT(TgcDigitJitterCondAlg)
DECLARE_COMPONENT(NswCalibDbAlg)
DECLARE_COMPONENT(NswDcsDbAlg)
DECLARE_COMPONENT(NswPassivationDbAlg)
DECLARE_COMPONENT(NswAsBuiltCondAlg)
DECLARE_COMPONENT(MdtAsBuiltCondAlg)
DECLARE_COMPONENT(CscILinesCondAlg)
DECLARE_COMPONENT(NswUncertDbAlg)
DECLARE_COMPONENT(sTGCAsBuiltCondAlg2)
DECLARE_COMPONENT(MmCTPCondDbAlg)