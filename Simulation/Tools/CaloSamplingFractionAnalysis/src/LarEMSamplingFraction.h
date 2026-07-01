/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LAREMSAMPLINGFRACTION_H
#define LAREMSAMPLINGFRACTION_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArElecCalib/ILArfSampl.h"

#include "TileConditions/TileSamplingFraction.h"
#include "TileConditions/TileCablingSvc.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Rtypes.h" //Long64_t
#include <vector>
#include <string>

class TTree;
class TileID;
class TileHWID;
class CaloCell_ID;

class LarEMSamplingFraction  : public ::AthAlgorithm
{

 public:

  LarEMSamplingFraction(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~LarEMSamplingFraction();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private:
  
  /// Default constructor: 
  LarEMSamplingFraction();
  
  bool m_docells{true};
  
  int m_mc_pdg{0};
  double m_mc_eta{0.};
  double m_mc_phi{0.};
  double m_mc_e{0.};
  double m_mc_pt{0.};
  
  std::vector<Long64_t>* m_cell_identifier{nullptr};
  std::vector<float>*    m_cell_energy_reco{nullptr};
  std::vector<float>*    m_cell_energy_active_total_corrected{nullptr};
  std::vector<float>*    m_cell_energy_active_total{nullptr};
  std::vector<float>*    m_cell_energy_inactive_total{nullptr};
  std::vector<int>*      m_cell_sampling{nullptr};
  std::vector<float>*    m_cell_eta{nullptr};
  std::vector<float>*    m_cell_phi{nullptr};

  std::vector<float> *m_energy_reco{nullptr};
  std::vector<float> *m_energy_hit{nullptr};

  std::vector<float> *m_energy_inactive_total{nullptr};
  std::vector<float> *m_energy_inactive_em{nullptr};
  std::vector<float> *m_energy_inactive_nonem{nullptr};
  std::vector<float> *m_energy_inactive_inv{nullptr};
  std::vector<float> *m_energy_inactive_esc{nullptr};
  
  std::vector<float> *m_energy_active_total_corrected{nullptr};
  std::vector<float> *m_energy_active_total{nullptr};
  std::vector<float> *m_energy_active_em{nullptr};
  std::vector<float> *m_energy_active_nonem{nullptr};
  std::vector<float> *m_energy_active_inv{nullptr};
  std::vector<float> *m_energy_active_esc{nullptr};
  
  TTree* m_mytree{nullptr};

  std::vector<std::string> m_CalibrationHitContainerNames;

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };
  
  const CaloCell_ID* m_calo_id{nullptr};
  
  const TileID * m_tileID{nullptr};
  const TileHWID* m_tileHWID{nullptr};
  const TileCablingService* m_tileCabling{nullptr};

  SG::ReadCondHandleKey<ILArfSampl> m_fSamplKey{this,"fSamplKey","LArfSamplSym","SG Key of LArfSampl object"};

  /**
   * @brief Name of TileSamplingFraction in condition store
   */
  SG::ReadCondHandleKey<TileSamplingFraction> m_tileSamplingFractionKey{this,
      "TileSamplingFraction", "TileSamplingFraction", "Input Tile sampling fraction"};

  /**
   * @brief Name of Tile cabling service
   */
  ServiceHandle<TileCablingSvc> m_tileCablingSvc{ this,
     "TileCablingSvc", "TileCablingSvc", "Tile cabling service"};
};

#endif //> !LarEMSamplingFraction_H
