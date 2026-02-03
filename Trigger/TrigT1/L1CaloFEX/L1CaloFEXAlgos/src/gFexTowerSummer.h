/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           gFexTowerSummer  -  description
//                              -------------------
//        Builds gFexDataTowers50 and gFexDataTowers200 from gFexDataTowers
//
//     begin                : 25 06 2025
//     email                : jared.little@cern.ch
//***************************************************************************/

#ifndef gFexTowerSummer_H
#define gFexTowerSummer_H

#include "AsgTools/ToolHandle.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODTrigL1Calo/TriggerTowerContainer.h"
#include "xAODTrigL1Calo/gFexTowerAuxContainer.h"
#include "xAODTrigL1Calo/gFexTowerContainer.h"

#include "L1CaloFEXByteStream/gFexPos.h"

#include <string>
#include <array>

class EventContext;


namespace LVL1 {

class gFexTowerSummer : public AthReentrantAlgorithm {
 public:
  gFexTowerSummer(const std::string& name, ISvcLocator* svc);

  /// Function initialising the algorithm
  virtual StatusCode initialize() override;
  /// Function executing the algorithm
  virtual StatusCode execute(const EventContext&) const override;

  typedef  std::array<std::array<int, LVL1::gFEXPos::AB_COLUMNS>, LVL1::gFEXPos::ABC_ROWS> gtFPGA;

  
 private:
  // ------------------------- Properties --------------------------------------
  
  // Read handle for gFex Data Fiber Towers
  SG::ReadHandleKey<xAOD::gFexTowerContainer> m_gFexFiberTowersReadKey{
      this, "gFexDataTowers", "L1_gFexDataTowers","gFexDataTowers container"};

  // Write handle for Data Towers
  SG::WriteHandleKey<xAOD::gFexTowerContainer> m_gTowersWriteKey{
      this, "gTowers200WriteKey", "L1_gFexDataTowers200","Write gFEX 200 MeV Trigger Tower container"};

  SG::WriteHandleKey<xAOD::gFexTowerContainer> m_gTowers50WriteKey{
    this, "gTowers50WriteKey", "L1_gFexDataTowers50", "Write gFEX 50 MeV Trigger Tower container"};

  // Optionally write the towers separated by EM and HAD still to be implemented later
  SG::WriteHandleKey<xAOD::gFexTowerContainer> m_gTowersEMWriteKey{
      this, "gTowersEMWriteKey", "L1_gFexEmulatedEMTowers", "Write gFEX 200 MeV Trigger Tower EM container"};

  SG::WriteHandleKey<xAOD::gFexTowerContainer> m_gTowersHADWriteKey{
      this, "gTowersHADWriteKey", "L1_gFexEmulatedHADTowers", "Write gFEX 200 MeV Trigger Tower HAD container"};  
  
  // originally from bytestream conversion
  // reconstruct gTowers from Fiber Towers
  StatusCode gtReconstructABC(const EventContext& ctx,
			      unsigned int XFPGA, 
			      gtFPGA &XgtF, gtFPGA &Xgt,
			      gtFPGA &Xsaturation) const;

  void undoMLE(int &datumPtr) const;
  void signExtend(int *xptr, int upto) const;
  void getEtaPhi(float& Eta, float& Phi, int iEta, int iPhi,
		 int gFEXtowerID) const;

};
}  // namespace LVL1
#endif
