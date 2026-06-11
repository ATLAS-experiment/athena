/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************
//  Filename : CalibHitIDCheck.h
//
//  Author   : gia.khoriauli@cern.ch
//  Created  : April, 2005
//
//  Helper tool for CalibHits Identifiers checking
//**********************************************************************

#ifndef CALOCALIBHITREC_CALIBHITIDCHECK_H
#define CALOCALIBHITREC_CALIBHITIDCHECK_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "Identifier/Identifier.h"

#include <vector>
#include <string>

class AtlasDetectorID;
class CaloCalibrationHitContainer;

class CalibHitIDCheck : public AthAlgorithm {

 public:

  CalibHitIDCheck(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~CalibHitIDCheck();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

  void check(int);
  void show_all();
  void check_all_cnts();
  void merge(Identifier);
  void reset(){m_id_vec.clear();}

 private:

  bool m_Merge{false};

  Gaudi::Property<std::string>  m_larDMHitContainer{this, "LArDMCalibHitCnt", "LArCalibrationHitDeadMaterial"};
  Gaudi::Property<std::string>  m_larActiveHitContainer{this, "ActiveCalibHitCnt", "LArCalibrationHitActive"};
  Gaudi::Property<std::string>  m_larInactiveHitContainer{this, "InactiveCalibHitCnt", "LArCalibrationHitInactive"};
  Gaudi::Property<std::string>  m_tileActiveHitContainer{this, "TileActiveHitCnt", "TileCalibHitActiveCell"};
  Gaudi::Property<std::string>  m_tileInactiveHitContainer{this, "TileInactiveHitCnt", "TileCalibHitInactiveCell"};
  Gaudi::Property<std::string>  m_tiledmHitContainer{this, "TileDMCalibHitCnt", "TileCalibHitDeadMaterial"};

  const CaloCalibrationHitContainer* m_LArDMHitCnt{};
  const CaloCalibrationHitContainer* m_ActiveHitCnt{};
  const CaloCalibrationHitContainer* m_InactiveHitCnt{};
  const CaloCalibrationHitContainer* m_TileActiveHitCnt{};
  const CaloCalibrationHitContainer* m_TileInactiveHitCnt{};
  const CaloCalibrationHitContainer* m_TileDMHitCnt{};

  Gaudi::Property<bool> m_Check{this, "Check", true};
  Gaudi::Property<bool> m_ShowAll{this, "ShowAll", false};
  Gaudi::Property<bool> m_CheckAll{this, "CheckAll", false};

  const AtlasDetectorID* m_id_helper{};

  std::vector<Identifier> m_id_vec;
};

#endif
