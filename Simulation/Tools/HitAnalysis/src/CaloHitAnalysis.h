/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HITANALYSIS_CALOHITANALYSIS_H
#define HITANALYSIS_CALOHITANALYSIS_H

// Base class
#include "AthenaBaseComps/AthAlgorithm.h"

// Members
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"

// STL includes
#include <string>

class TileID;
class TileDetDescrManager;
class TH1;
class TH2;
class TTree;


class CaloHitAnalysis : public AthAlgorithm {

public:

  CaloHitAnalysis(const std::string& name, ISvcLocator* pSvcLocator);
  ~CaloHitAnalysis() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;

private:

  /** Simple variables by Ketevi */
  TH1* m_h_cell_eta{};
  TH1* m_h_cell_phi{};
  TH1* m_h_cell_e{};
  TH1* m_h_cell_radius{};
  TH2* m_h_xy{};
  TH2* m_h_zr{};
  TH2* m_h_etaphi{};
  TH2* m_h_time_e{};
  TH2* m_h_eta_e{};
  TH2* m_h_phi_e{};
  TH2* m_h_r_e{};
  TH1* m_h_calib_eta{};
  TH1* m_h_calib_phi{};
  TH2* m_h_calib_rz{};
  TH2* m_h_calib_etaphi{};
  TH1* m_h_calib_eEM{};
  TH1* m_h_calib_eNonEM{};
  TH1* m_h_calib_eInv{};
  TH1* m_h_calib_eEsc{};
  TH1* m_h_calib_eTot{};
  TH1* m_h_calib_eTotpartID{};

  const TileID * m_tileID{};
  const TileDetDescrManager * m_tileMgr{};

  std::vector<float>* m_cell_eta{};
  std::vector<float>* m_cell_phi{};
  std::vector<float>* m_cell_x{};
  std::vector<float>* m_cell_y{};
  std::vector<float>* m_cell_z{};
  std::vector<float>* m_cell_e{};
  std::vector<float>* m_cell_radius{};
  std::vector<float>* m_time{};
  std::vector<float>* m_calib_eta{};
  std::vector<float>* m_calib_phi{};
  std::vector<float>* m_calib_radius{};
  std::vector<float>* m_calib_z{};
  std::vector<float>* m_calib_eEM{};
  std::vector<float>* m_calib_eNonEM{};
  std::vector<float>* m_calib_eInv{};
  std::vector<float>* m_calib_eEsc{};
  std::vector<float>* m_calib_eTot{};
  std::vector<float>* m_calib_partID{};

  BooleanProperty m_expert{this, "ExpertMode", false};
  BooleanProperty m_calib{this, "UseCalibHits", false};
  BooleanProperty m_useLAr{this, "UseLAr", true};
  BooleanProperty m_useTile{this, "UseTile", true};

  TTree* m_tree{};
  StringProperty m_ntupleFileName{this, "NtupleFileName", "/CaloHitAnalysis/"};
  StringProperty m_path{this, "HistPath", "/CaloHitAnalysis/"};
  ServiceHandle<ITHistSvc> m_thistSvc{this, "THitSvc", "THistSvc"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
    , "CaloDetDescrManager"
    , "CaloDetDescrManager"
    , "SG Key for CaloDetDescrManager in the Condition Store" };

};

#endif // CALO_HIT_ANALYSIS_H
