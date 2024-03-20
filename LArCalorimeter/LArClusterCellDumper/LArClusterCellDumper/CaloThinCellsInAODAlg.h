// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file LArClusterCellDumper/src/CaloThinCellsInAODAlg.h
 * @author mateus hufnagel <mateus.hufnagel@cern.ch>
 * @date mar, 2024
 * @brief Thin calorimeter cells, digits and raw channels associated with clusters, after some cut, and forward them to AOD.
 */

#ifndef LARCLUSTERCELLDUMPER_CALOTHINCELLSINAODALG_H
#define LARCLUSTERCELLDUMPER_CALOTHINCELLSINAODALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthContainers/ConstDataVector.h"
#include "StoreGate/ReadHandleKey.h"

#include "LArIdentifier/LArOnlineID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "LArCabling/LArOnOffIdMapping.h"

#include "LArSimEvent/LArHitContainer.h"
#include "CaloEvent/CaloCellContainer.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

/**
 * @brief Thin calorimeter cells, digits and raw channels not associated with clusters linked to an electron, after some cut.
 *
 * Thinning algorithm to keep calorimeter cells, raw channels and digits.
 * The selection is done for cells linked to calo clusters, with eta and pt cuts.
 */
class CaloThinCellsInAODAlg : public AthReentrantAlgorithm
{
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

  const LArOnlineID* m_onlineID = nullptr;
  const CaloCell_ID* m_caloCellId  = nullptr;

  typedef ConstDataVector<LArHitContainer>   ConstLArHitCont_t;
  typedef ConstDataVector<LArDigitContainer> ConstLArDigitCont_t;
  typedef ConstDataVector<CaloCellContainer> ConstCaloCellCont_t;

  Gaudi::Property<float> m_clusterPtCut {this, "ClusterPtCut", 1000, "Cluster pt cut in MeV."};
  Gaudi::Property<float> m_clusterEtaCut {this, "ClusterEtaCut", 1.4, "Cluster abs(eta) cut."};
  Gaudi::Property<bool>  m_isMC {this, "isMC", false, "Input data is MC."};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};

  // Name of the containers being thinned.
  SG::ReadHandleKey<LArHitContainer> m_hitsInputKey{this, "InputHitsContainerName","LArHitEMB", "SG Key of LArHitsContainer"};
  SG::ReadHandleKey<LArDigitContainer> m_digitsInputKey{this, "InputDigitsContainerName","LArDigitContainer_MC", "SG Key of LArDigitContainer"};
  SG::ReadHandleKey<LArRawChannelContainer> m_rawChInputKey{this, "InputRawChannelContainerName","LArRawChannels", "SG Key of LArRawChannel container"};
  SG::ReadHandleKey<CaloCellContainer> m_caloCellInputKey{this, "InputCaloCellContainerName","AllCalo", "SG Key of CaloCell container"};

  // Cluster container to be used as reference for cells being thinned
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterCntKey{this, "CaloClusterContainerKey", "CaloCalTopoClusters", "Name of the Electrons Container"};

  // Output containers
  SG::WriteHandleKey<LArHitContainer> m_hitsOutputKey{this, "OutputHitsContainerName","LArHitEMB_ClusterThinned", "SG Key of thinned LArHitEMB container"};
  SG::WriteHandleKey<ConstLArDigitCont_t> m_digitsOutputKey{this, "OutputDigitsContainerName","LArDigitContainer_ClusterThinned", "SG Key of thinned LArDigitContainer"};
  SG::WriteHandleKey<LArRawChannelContainer> m_rawChOutputKey{this, "OutputRawChannelContainerName","LArRawChannels_ClusterThinned", "SG Key of LArRawChannel container"};
  SG::WriteHandleKey<ConstCaloCellCont_t> m_caloCellOutputKey{this, "OutputCaloCellContainerName","AllCalo_ClusterThinned", "SG Key of CaloCell container"};

};


#endif