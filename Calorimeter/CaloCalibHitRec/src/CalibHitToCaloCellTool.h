/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 *
 * @file   CalibHitToCaloCellTool.h
 * @author Ioannis Nomidis <ioannis.nomidis@cern.ch>
 * @date   Dec 2016
 * @brief  Convert energy deposits from calibration hits to CaloCell, xAOD::CaloCluster
 */

#ifndef CALOCALIBHITREC_CALIBHITTOCALOCELLTOOL_H
#define CALOCALIBHITREC_CALIBHITTOCALOCELLTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteHandleKeyArray.h"

#include <string>
#include <vector>

class CaloCell_ID;
class CaloDM_ID;

class CaloCell;
class CaloCellContainer;
class CaloCellLinkContainer;

static const InterfaceID IID_CalibHitToCaloCellTool("CalibHitToCaloCellTool", 1, 0);

namespace CalibHitUtils {
  enum EnergyType { EnergyEM=0, EnergyVisible, EnergyTotal, nEnergyTypes };  
}

class CalibHitToCaloCellTool: virtual public AthAlgTool {

 public:
  CalibHitToCaloCellTool(const std::string& t, const std::string& n, const IInterface*  p);
  ~CalibHitToCaloCellTool();
  virtual StatusCode initialize() override;
  StatusCode processCalibHitsFromParticle() const;
  
  static const InterfaceID& interfaceID() { return IID_CalibHitToCaloCellTool;}

 private:
  Gaudi::Property<int> m_caloGain{this, "CaloGain", static_cast<int>(CaloGain::LARLOWGAIN)};
  Gaudi::Property<std::vector<std::string>> m_calibHitContainerNames{this, "CalibHitContainers", {}};
  
  std::string m_tileActiveHitCnt{"TileCalibHitActiveCell"};
  std::string m_tileInactiveHitCnt{"TileCalibHitInactiveCell"};
  std::string m_tileDMHitCnt{"TileCalibHitDeadMaterial"};
  std::string m_larInactHitCnt{"LArCalibrationHitActive"};
  std::string m_larActHitCnt{"LArCalibrationHitInactive"};
  std::string m_larDMHitCnt{"LArCalibrationHitDeadMaterial"};

  Gaudi::Property<bool> m_doTile{this, "DoTile", false};

  Gaudi::Property<std::string> m_caloCell_Tot{this, "CellTotEne", "TotalCalibCell"};
  Gaudi::Property<std::string> m_caloCell_Vis{this, "CellVisEne", "VisCalibCell"};
  Gaudi::Property<std::string> m_caloCell_Em{this, "CellEmEne", ""};
  Gaudi::Property<std::string> m_caloCell_NonEm{this, "CellNonEmEne", ""};

  const CaloCell_ID*  m_caloCell_ID{nullptr};
  const CaloDM_ID*    m_caloDM_ID{nullptr};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

  Gaudi::Property<std::string> m_outputCellContainerName{this, "OutputCellContainerName", "TruthCells"};
  Gaudi::Property<std::string> m_outputClusterContainerName{this, "OutputClusterContainerName", "TruthClusters"};
  
  SG::WriteHandleKeyArray<CaloCellContainer> m_cellContKeys;
  SG::WriteHandleKeyArray<xAOD::CaloClusterContainer> m_clusterContKeys;
  SG::WriteHandleKeyArray<CaloClusterCellLinkContainer> m_cellLinkKeys;

  const std::array<std::string, 3> m_energyTypeToStr{"Eem","Evis","Etot"};  
};

#endif
