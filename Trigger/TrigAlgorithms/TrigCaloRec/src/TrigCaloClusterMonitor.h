//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef TRIGCALOREC_TRIGCALOCLUSTERMONITOR_H
#define TRIGCALOREC_TRIGCALOCLUSTERMONITOR_H

/********************************************************************
 *
 * NAME:      TrigCaloClusterMonitor
 * PACKAGE:   Trigger/TrigAlgorithms/TrigCaloRec
 *
 * AUTHOR:    Nuno Fernandes
 * CREATED:   September 2024
 *
 *          Stores the relevant information for monitoring topological clustering
 *          through monitored variables.
 *********************************************************************/


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"


class TrigCaloClusterMonitor : public AthReentrantAlgorithm
{

 public:

  TrigCaloClusterMonitor(const std::string & name, ISvcLocator * pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext & ctx) const override;

 private:
  
  Gaudi::Property<bool> m_monitorCells{this, "MonitorCells", false, "Whether to monitor cells too."};
  
  Gaudi::Property<bool> m_excludeTile{this, "ExcludeTileCells", true, "Whether to exclude TileCal cells from monitoring."};
  Gaudi::Property<bool> m_useTwoGaussianNoise{this, "TwoGaussianNoise", false, "Use 2-gaussian noise description for TileCal."};
  
  
  Gaudi::Property<float> m_monitoring1thr { this, "Thr1", 2, "First Threshold to pass for cell monitoring" };
  Gaudi::Property<float> m_monitoring2thr { this, "Thr2", 4, "Second Threshold to pass for cell monitoring" };
  
  
  Gaudi::Property<unsigned int> m_monitorInterval { this, "MonitoringInterval", 1, "Monitor just one every MonitoringInterval events, in case statistics are high enough." };
  
  Gaudi::Property<unsigned int> m_monitorCellsInterval { this, "MonitoringCellInterval", 20, "Additional interval in which to monitor the cells, since this is a costlier monitoring." };
 
  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};
  
  /** @brief Monitoring tool.
    */
  ToolHandle<GenericMonitoringTool> m_moniTool { this, "MonitoringTool", "", "Monitoring tool" };

  ///Event input: To get <mu> from Event Info
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_avgMuKey { this, "averageInteractionsPerCrossingKey", "EventInfo.averageInteractionsPerCrossing", "Decoration for Average Interaction Per Crossing" };
  
  /**
   * @brief vector of names of the cell containers to use as input.
   */
  SG::ReadHandleKey<CaloCellContainer> m_cellsKey {this, "CellsName", "", "Name(s) of Cell Containers"};
  
  /** @brief The name of the key in StoreGate for the CaloClusterContainer we want to monitor */
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clustersKey {this, "ClustersName", "",
                                                               "The name of the key in StoreGate for the CaloClusterContainer we want to monitor."};
};

#endif
