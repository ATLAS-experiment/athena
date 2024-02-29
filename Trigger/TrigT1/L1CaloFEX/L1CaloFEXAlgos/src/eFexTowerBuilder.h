/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTowerBuilder  -  description:
//       Builds an eFexTowerContainer from a CaloCellContainer (for supercells) and TriggerTowerContainer (for ppm tile towers)
//                              -------------------
//     begin                : 06 12 2022
//     email                : will@cern.ch
//***************************************************************************/

#ifndef eFexTowerBuilder_H
#define eFexTowerBuilder_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODTrigL1Calo/eFexTowerContainer.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRecConditions/LArBadChannelCont.h"
//#include "LArCabling/LArOnOffIdMapping.h"

#include "L1CaloFEXSim/eFEXSuperCellTowerIdProvider.h"

class CaloIdManager;


/**
 * eFexTowerBuilder creates xAOD::eFexTowerContainer from
 * supercells (LATOME) and triggerTowers (TREX) inputs.
 *
 * All the created towers have module=-1 and fpga=-1 and there's only one
 * tower for each eta/phi location.
 *
 */
namespace LVL1 {

class eFexTowerBuilder : public AthReentrantAlgorithm
{
 public:

  eFexTowerBuilder(const std::string& name, ISvcLocator* pSvcLocator);
  ~eFexTowerBuilder() = default;

  virtual StatusCode initialize();
  virtual StatusCode execute(const EventContext& ctx) const;

 private:
    mutable std::mutex m_fillMapMutex ATLAS_THREAD_SAFE;
    mutable std::map<unsigned long long, std::pair<std::pair<int,int>,std::pair<int,int>>> m_scMap ATLAS_THREAD_SAFE; // maps from scid -> (efex eta/phi index pair, slot pair)

    StatusCode fillTowers(const EventContext& ctx) const;
    StatusCode fillMap(const EventContext& ctx) const;

    SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_ddmKey{this,"CaloSuperCellDetDescrManager","CaloSuperCellDetDescrManager",""};

  SG::ReadHandleKey<CaloCellContainer> m_scellKey { this, "CaloCellContainerReadKey", "SCell", "Read handle key for the supercells"};
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_ttKey { this, "TriggerTowerContainerReadKey", "xAODTriggerTowers", "Read handle key for the triggerTowers"};
  SG::WriteHandleKey<xAOD::eFexTowerContainer> m_outKey {this, "eFexContainerWriteKey", "L1_eFexEmulatedTowers", "Name of the output container"};

    Gaudi::Property<std::string> m_mappingFile {this, "MappingFile", "L1CaloFEXByteStream/2023-02-13/scToEfexTowers.root", "PathResolver location to mapping file"};
    ToolHandle<IeFEXSuperCellTowerIdProvider> m_eFEXSuperCellTowerIdProviderTool {this, "eFEXSuperCellTowerIdProviderTool", "LVL1::eFEXSuperCellTowerIdProvider", "Tool that provides tower-FOGA mapping"};

    Gaudi::Property<bool> m_applyMasking{this,"ApplyMasking",true,"Apply masking of supercells based on provenance bits. Should be set to False for MC"};

    mutable std::mutex m_debugMutex;

};

} // end of LVL1 namespace
#endif
