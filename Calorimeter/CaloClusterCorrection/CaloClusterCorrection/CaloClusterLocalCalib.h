/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//Dear emacs, this is -*-c++-*-

#ifndef CALOUTILS_CALOCLUSTERLOCALCALIB_H
#define CALOUTILS_CALOCLUSTERLOCALCALIB_H 


#include "CaloUtils/CaloClusterProcessor.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloEvent/CaloRecoStatus.h"
#include "CaloInterface/IClusterCellWeightTool.h"
#include "CaloInterface/IClusterClassificationTool.h"

#include "GaudiKernel/ToolHandle.h"

/*!
\file  CaloClusterLocalCalib.h
\author   Kai Voss
\date     September 2005

\class CaloClusterLocalCalib
\brief Cluster correction tool which applies local hadronic calibration  w
       eights to the cells. The weight are provided by ICellWeight
       tools

*/


class CaloClusterLocalCalib : public CaloClusterProcessor {

 public:
  
  /// Tool initialization: load calibration tools specified by jobOptions 
  virtual StatusCode initialize() override;

  /// Apply corrections to cluster
  using CaloClusterProcessor::execute;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloCluster* theCluster) const override;

  /// Standard AlgTool constructor
  CaloClusterLocalCalib(const std::string& type,
 		        const std::string& name,
	 	        const IInterface* parent);
  
 private:

  /// property:  Classification tools
  ToolHandleArray<IClusterClassificationTool>  m_classificationTool{this, "ClusterClassificationTool", {}};;
  //Remark: This handle should be 0 or 1 entries. Our configurable framework can't handle 
  //a not-assigned ToolHandle, therefore this is an array.
  
  /// property: Array of IClusterCellWeightTool 
  ToolHandleArray<IClusterCellWeightTool> m_calibTools{this, "LocalCalibTools", {}};
  //  std::vector < IClusterCellWeightTool *> m_calibToolPointers ;

  /// property: vector of valid Reco Statuses for the clusters in
  /// order to be calibrated
  Gaudi::Property<std::vector<int>> m_recoStatus{this, "ClusterRecoStatus", {}};
  
  /// if set to true, negative clusters are weighted as well
  Gaudi::Property<bool> m_absOpt{this, "WeightingOfNegClusters", false};  
};


#endif
