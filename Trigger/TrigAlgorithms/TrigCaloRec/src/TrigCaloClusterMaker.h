
// Hi Emacs ! this is  -*- C++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/********************************************************************
 *
 * NAME:      TrigCaloClusterMaker
 * PACKAGE:   Trigger/TrigAlgorithms/TrigCaloRec
 *
 * AUTHOR:    P.A. Delsart
 * CREATED:   August 2006
 *
 *          Builds Clusters from CaloCell in a ROI using offline calo tools.
 *          Works currently only for CaloTopoCluster and SWClusters. (hard coded hack...)
 *********************************************************************/
#ifndef TRIGCALOREC_TRIGCALOCLUSTERMAKER_H
#define TRIGCALOREC_TRIGCALOCLUSTERMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "CaloUtils/CaloClusterProcessor.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"


class TrigCaloClusterMaker : public AthReentrantAlgorithm {

 public:

  TrigCaloClusterMaker(const std::string& name, ISvcLocator* pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:

  ToolHandleArray<CaloClusterCollectionProcessor> m_clusterMakers
  { this, "ClusterMakerTools", {}, "" };
  ToolHandleArray<CaloClusterProcessor> m_clusterCorrections
  { this, "ClusterCorrectionTools", {}, "" };

  // Following used for testing only :
  //bool        m_useMeaningfullNames;      
  std::string m_clustersOutputName;

  SG::ReadHandleKey<CaloCellContainer> m_inputCellsKey{ this,
      "Cells",                  // property name
      "cells",                                             // default value of StoreGate key
      "input CaloCellContainer "};

  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outputClustersKey{ this,
      "CaloClusters",                  // property name
      "caloclusters",                                             // default value of StoreGate key
      "output CaloClusterContainer"};

  // This is to write out the cells container so it is available at later steps
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_clusterCellLinkOutput{ this,
      "CellLinks",
      "celllinks",
      "Output cell links"};


  ToolHandle< GenericMonitoringTool > m_monTool { this, "MonTool", "", "Monitoring tool" };

  Gaudi::Property<SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> > m_mDecor_ncells { this,
      "Decor_ncells",                // decorator name
      "nCells",                      // default value
      "Decorator containing the number of cells associated to a cluster"};

  // adding noise handle for monitoring purposes
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};
  // Still for monitoring purposes
  Gaudi::Property<bool>  m_monCells { this, "MonCells", false, "Do I monitor the cells I receive" };
  Gaudi::Property<float> m_1thr { this, "Thr1", 2, "First Threshold to pass" };
  Gaudi::Property<float> m_2thr { this, "Thr2", 4, "Second Threshold to pass" };
  //Event input: To get <mu> from Event Info
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_avgMuKey { this, "averageInteractionsPerCrossingKey", "EventInfo.averageInteractionsPerCrossing", "Decoration for Average Interaction Per Crossing" };

  bool m_isSW{false};
};
#endif
