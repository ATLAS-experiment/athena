/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEF_TRIGCALOEVENT
#define TRIGSTORAGEDEF_TRIGCALOEVENT
#include "TrigStorageDefinitions/EDM_TypeInformation.h"
#include "Navigation/NavigationDefaults.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloCellLinkContainer.h"
#include "CaloEvent/CaloClusterContainer.h"
#include "CaloEvent/CaloTowerContainer.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"

#include "xAODTrigCalo/TrigCaloCluster.h"
#include "xAODTrigCalo/TrigCaloClusterContainer.h"
#include "xAODTrigCalo/TrigCaloClusterAuxContainer.h"

#include "xAODTrigCalo/CaloClusterTrigAuxContainer.h"

#include "xAODTrigCalo/TrigEMCluster.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTrigCalo/TrigEMClusterAuxContainer.h"

#include "xAODTrigMinBias/TrigT2MbtsBits.h"
#include "xAODTrigMinBias/TrigT2MbtsBitsContainer.h"
#include "xAODTrigMinBias/TrigT2MbtsBitsAuxContainer.h"

#include "xAODTrigMinBias/TrigT2ZdcSignals.h"
#include "xAODTrigMinBias/TrigT2ZdcSignalsContainer.h"
#include "xAODTrigMinBias/TrigT2ZdcSignalsAuxContainer.h"


#include "xAODTrigRinger/TrigRingerRings.h"
#include "xAODTrigRinger/TrigRingerRingsContainer.h"
#include "xAODTrigRinger/TrigRingerRingsAuxContainer.h"

#include "xAODTrigRinger/TrigRNNOutput.h"
#include "xAODTrigRinger/TrigRNNOutputContainer.h"
#include "xAODTrigRinger/TrigRNNOutputAuxContainer.h"


#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerAuxContainer.h"

#include "xAODHIEvent/HIEventShapeContainer.h"
#include "xAODHIEvent/HIEventShapeAuxContainer.h"



HLT_BEGIN_TYPE_REGISTRATION
  //EF
  HLT_REGISTER_TYPE(class CaloCell,class  CaloCellContainer,class CaloCellContainer )
  HLT_REGISTER_TYPE(class CaloCellLink,class  CaloCellLinkContainer,class CaloCellLinkContainer )
  HLT_REGISTER_TYPE(class CaloCluster, class CaloClusterContainer, class CaloClusterContainer )
  HLT_REGISTER_TYPE(class CaloClusterCellLink, class CaloClusterCellLinkContainer, class CaloClusterCellLinkContainer)
  HLT_REGISTER_TYPE(class CaloTower, class  CaloTowerContainer, class CaloTowerContainer )

  // xAOD
  HLT_REGISTER_TYPE(xAOD::CaloCluster, xAOD::CaloClusterContainer, xAOD::CaloClusterContainer, xAOD::CaloClusterTrigAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigCaloCluster, xAOD::TrigCaloCluster, xAOD::TrigCaloClusterContainer, xAOD::TrigCaloClusterAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigEMCluster, xAOD::TrigEMCluster, xAOD::TrigEMClusterContainer, xAOD::TrigEMClusterAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigEMCluster, xAOD::TrigEMClusterContainer, xAOD::TrigEMClusterContainer, xAOD::TrigEMClusterAuxContainer) 

  HLT_REGISTER_TYPE(xAOD::TrigT2MbtsBits, xAOD::TrigT2MbtsBits, xAOD::TrigT2MbtsBitsContainer, xAOD::TrigT2MbtsBitsAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigT2ZdcSignals, xAOD::TrigT2ZdcSignals, xAOD::TrigT2ZdcSignalsContainer, xAOD::TrigT2ZdcSignalsAuxContainer)

  HLT_REGISTER_TYPE(xAOD::TrigRingerRings, xAOD::TrigRingerRings, xAOD::TrigRingerRingsContainer, xAOD::TrigRingerRingsAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TrigRNNOutput, xAOD::TrigRNNOutput, xAOD::TrigRNNOutputContainer, xAOD::TrigRNNOutputAuxContainer)
  HLT_REGISTER_TYPE(xAOD::TriggerTower, xAOD::TriggerTowerContainer, xAOD::TriggerTowerContainer, xAOD::TriggerTowerAuxContainer)
  HLT_REGISTER_TYPE(xAOD::HIEventShape, xAOD::HIEventShapeContainer, xAOD::HIEventShapeContainer, xAOD::HIEventShapeAuxContainer)
HLT_END_TYPE_REGISTRATION(TrigCaloEvent)



#endif
