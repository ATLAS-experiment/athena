/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 *
 * @file   CalibHitToCaloCell.h
 * @author Gia, gia@mail.cern.ch
 * @date   March, 2005
 */

#ifndef CALOCALIBHITREC_CALIBHITTOCALOCELL_H
#define CALOCALIBHITREC_CALIBHITTOCALOCELL_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"

#include <string>
#include <vector>

class CaloCell_ID;
class CaloDM_ID;

class LArCell;

class CalibHitToCaloCell : public AthAlgorithm {
public:

  CalibHitToCaloCell(const std::string& name, ISvcLocator* pSvcLocator);

  virtual ~CalibHitToCaloCell();                         
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  
private:
  std::string m_tileActiveHitCnt{"TileCalibHitActiveCell"};
  std::string m_tileInactiveHitCnt{"TileCalibHitInactiveCell"};
  std::string m_tileDMHitCnt{"TileCalibHitDeadMaterial"};
  std::string m_larInactHitCnt{"LArCalibrationHitInactive"};
  std::string m_larActHitCnt{"LArCalibrationHitActive"};
  std::string m_larDMHitCnt{"LArCalibrationHitDeadMaterial"};
    
  bool m_store_Tot{false};
  bool m_store_Vis{false};
  bool m_store_Em{false};
  bool m_store_NonEm{false};

  Gaudi::Property<bool> m_storeUnknown{this, "StoreUnknownCells", false};

  Gaudi::Property<std::string> m_caloCell_Tot{this, "CellTotEne", "TotalCalibCell"};
  Gaudi::Property<std::string> m_caloCell_Vis{this, "CellVisEne", "VisCalibCell"};
  Gaudi::Property<std::string> m_caloCell_Em{this, "CellEmEne", ""};
  Gaudi::Property<std::string> m_caloCell_NonEm{this, "CellNonEmEne", ""};
  
  const CaloCell_ID*  m_caloCell_ID{nullptr};
  const CaloDM_ID*    m_caloDM_ID{nullptr};
  
  std::vector<LArCell*> m_Cells_Tot;
  std::vector<LArCell*> m_Cells_Vis;
  std::vector<LArCell*> m_Cells_Em;
  std::vector<LArCell*> m_Cells_NonEm;
  
  int m_nchan{0};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
    , "CaloDetDescrManager"
    , "CaloDetDescrManager"
    , "SG Key for CaloDetDescrManager in the Condition Store" };
};

#endif
