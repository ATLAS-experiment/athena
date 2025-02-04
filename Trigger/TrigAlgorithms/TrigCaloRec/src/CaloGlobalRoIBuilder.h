
/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/********************************************************************
 * 
 *   NAME:      TrigCaloClusterMaker
 *   PACKAGE:   Trigger/TrigAlgorithms/TrigCaloRec
 *  
 *   AUTHOR:    D.O. Damazio
 *   CREATED:   October 2023
 *   
 *   Builds RoIs from CaloCell, can also produce further cluster
 *********************************************************************/
#ifndef TRIGCALOREC_CALOGLOBALROIBUILDER_H
#define TRIGCALOREC_CALOGLOBALROIBUILDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloConditions/CaloNoise.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrigT2CaloEgamma/RingerReFex.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTrigRinger/TrigRingerRingsContainer.h"


class CaloGlobalRoIBuilder :  public AthReentrantAlgorithm {
  public:
    CaloGlobalRoIBuilder(const std::string& name, ISvcLocator* pSvcLocator);
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

  private:
    SG::ReadHandleKey<CaloCellContainer> m_inputCellsKey{ this,
      "Cells",                  // property name
      "cells",                  // default value of StoreGate key
      "input CaloCellContainer "};
    // adding noise handle for monitoring purposes
    SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};
    Gaudi::Property<float> m_thr { this, "Thr", 4, "Threshold to pass" };
    Gaudi::Property<float> m_abseta_thr { this, "AbsEtaThr", 2.5, "Threshold to define fiducial volume Abs Eta" };
    Gaudi::Property<float> m_samp_thr { this, "SampThr", 7, "Threshold to define fiducial volume Samp" }; // only EM

    ToolHandle<RingerReFex> m_emAlgTools{ this, "RingerTool", {}, "RingerReFex"};
    SG::WriteHandleKey<xAOD::TrigEMClusterContainer> m_clusterContainerKey{
         this, "ClustersName", "CaloClustersGlobal", "Calo cluster container"};
    SG::WriteHandleKey<xAOD::TrigRingerRingsContainer> m_ringerContainerKey  {this, "RingerKey", "Global_FastCaloRinger", "TrigRingerRings container key"};

};

#endif
