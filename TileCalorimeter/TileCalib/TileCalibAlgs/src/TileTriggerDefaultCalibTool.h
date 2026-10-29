/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECALIBALG_TILETRIGGERDEFAULTCALIBTOOL_H
#define TILECALIBALG_TILETRIGGERDEFAULTCALIBTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "TileCalibAlgs/ITileCalibTool.h"
#include "TrigT1CaloCalibToolInterfaces/IL1CaloTTIdTools.h" 
#include "TileEvent/TileDQstatus.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "StoreGate/ReadHandleKey.h"

#include <string> 

class TileCablingService;
class CaloLVL1_ID;
class TileHWID;
class TileID;
class TFile;
class TileRawChannelContainer;
class TileBeamElemContainer;
class Identifier;
class HWIdentifier;


class TileTriggerDefaultCalibTool : public AthAlgTool, virtual public ITileCalibTool
{

 public:
  TileTriggerDefaultCalibTool(const std::string& type, const std::string& name,const IInterface* pParent);
  virtual ~TileTriggerDefaultCalibTool();

  virtual StatusCode initialize() override;
  virtual StatusCode initNtuple(int runNumber, int runType, TFile * rootfile) override;
  virtual StatusCode execute() override;
  virtual StatusCode finalizeCalculations() override;
  virtual StatusCode writeNtuple(int runNumber, int runType, TFile * rootfile) override;
  virtual StatusCode finalize() override;

 private:

  // jobOptions
  Gaudi::Property<std::string> m_ntupleID{this, "NtupleID", "h3000"};
  Gaudi::Property<int> m_maxNTT{this, "MaxNTriggerTowers", 7200};
  Gaudi::Property<unsigned int> m_nevpmt{this, "NumEventPerPMT", 195}; // changed from 200 to 195

  // Tools / storegate info
  const CaloLVL1_ID* m_TT_ID{nullptr};
  const TileHWID* m_tileHWID{nullptr};
  const TileID*   m_tileID{nullptr};
  const TileCablingService* m_tileCablingService{nullptr};
  ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,  //!< main Tile Calibration tool
    "TileCondToolEmscale", "TileCondToolEmscale", "Tile em scale tool"};
  SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{this,
        "TileDQstatus", "TileDQstatus", "TileDQstatus key"};
  SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
      "TileRawChannelContainer", "TileRawChannelFit", "Tile raw channel container"};
  SG::ReadHandleKey<xAOD::TriggerTowerContainer> m_triggerTowerContainerKey{this,
      "TriggerTowerContainer", "xAODTriggerTowers", "Trigger Tower container"};
 
  ToolHandle<LVL1::IL1CaloTTIdTools > m_l1CaloTTIdTools{this,
    "L1CaloTTIdTools", "LVL1::L1CaloTTIdTools/L1CaloTTIdTools", "L1Calo TTId tools"};

  using Tile = TileCalibUtils;

  // Results Tile
  float (*m_meanTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_rmsTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_meanTileDAC)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_rmsTileDAC)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_ietaTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_iphiTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_ipmtTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_nEvtTile)[Tile::MAX_DRAWER][Tile::MAX_CHAN];

  // Results L1Calo
  float (*m_meanL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_rmsL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_meanL1CaloDAC)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_rmsL1CaloDAC)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_ietaL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_iphiL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_ipmtL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  int   (*m_nEvtL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];

  float (*m_meanTileL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];
  float (*m_rmsTileL1Calo)[Tile::MAX_DRAWER][Tile::MAX_CHAN];

  // CISpar parameters
  float 	m_charge{0};
  unsigned int	m_ipmt{0};
  unsigned int	m_ipmtCount{0};
  unsigned int	m_ipmtOld{0};

  float m_DACvalue{0};

  // Events
  int   m_nEvtGlobal{0};

};

#endif // #ifndef TILECALIBALG_TILETRIGGERDEFAULTCALIBTOOL_H
