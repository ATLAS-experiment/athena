/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_TRIGMUONEVENT
#define TRIGSTORAGEDEF_TRIGMUONEVENT
#include "TrigStorageDefinitions/EDM_TypeInformation.h"

#include "xAODMuon/MuonContainer.h"
#include "xAODMuon/MuonAuxContainer.h"
#include "xAODTrigMuon/L2StandAloneMuonContainer.h"
#include "xAODTrigMuon/L2StandAloneMuonAuxContainer.h"
#include "xAODTrigMuon/L2CombinedMuonContainer.h"
#include "xAODTrigMuon/L2CombinedMuonAuxContainer.h"
#include "xAODTrigMuon/L2IsoMuonContainer.h"
#include "xAODTrigMuon/L2IsoMuonAuxContainer.h"

HLT_BEGIN_TYPE_REGISTRATION
  HLT_REGISTER_TYPE(xAOD::Muon, xAOD::MuonContainer, xAOD::MuonContainer, xAOD::MuonAuxContainer)
  HLT_REGISTER_TYPE(xAOD::L2StandAloneMuon, xAOD::L2StandAloneMuonContainer, xAOD::L2StandAloneMuonContainer, xAOD::L2StandAloneMuonAuxContainer)
  HLT_REGISTER_TYPE(xAOD::L2CombinedMuon, xAOD::L2CombinedMuonContainer, xAOD::L2CombinedMuonContainer, xAOD::L2CombinedMuonAuxContainer)
  HLT_REGISTER_TYPE(xAOD::L2IsoMuon, xAOD::L2IsoMuonContainer, xAOD::L2IsoMuonContainer, xAOD::L2IsoMuonAuxContainer)
HLT_END_TYPE_REGISTRATION(TrigMuonEvent)

#endif
