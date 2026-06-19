/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFUNIFIEDMOMENTCALCULATORTOOL_H
#define PFUNIFIEDMOMENTCALCULATORTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include "IPFUnifiedBaseTool.h"
#include "IPFClusterCollectionTool.h"

struct PFData;

/**
This tool uses a CaloClusterCollectionProcessor to calculate new moments for the modified calorimeter clusters (modified when we do the charged shower subtraction), and makes use of IPFClusterCollectionTool to generate a VIEW container of xAOD::CaloCluster to be used in the CaloClusterCollectionProcessor tool. Inherits from IPFBaseAlgTool.
*/
class PFUnifiedMomentCalculatorTool : public extends<AthAlgTool, IPFUnifiedBaseTool> {

  public:
  
  PFUnifiedMomentCalculatorTool(const std::string& type,const std::string& name,const IInterface* parent);

  ~PFUnifiedMomentCalculatorTool() {}

  virtual StatusCode initialize() override;
  virtual StatusCode processPFlowData(const EventContext& ctx, PFData &thePFData) const override;

 private:

  /** Tool to put all clusters into a temporary container - then we use this to calculate moments, some of which depend on configuration of nearby clusters */
  ToolHandle<IPFClusterCollectionTool> m_clusterCollectionTool{this,"PFClusterCollectionTool","eflowRecClusterCollectionTool","Tool to put all clusters into a temporary container - then we use this to calculate moments, some of which depend on configuration of nearby clusters"};
  
  /** Tool to calculate cluster moments */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterMomentsMaker{this,"CaloClusterMomentsMaker","CaloClusterMomentsMaker","Tool to calculate cluster moments"};

  /** Tool to calculate calibration hit truth cluster moments */
  ToolHandle<CaloClusterCollectionProcessor> m_clusterCalibHitMomentsMaker2{this,"CaloCalibClusterMomentsMaker2","CaloCalibClusterMomentsMaker2","Tool to calculate calibration hit cluster moments"};

  /** Toggle whether we are in LC mode - false by default */
  Gaudi::Property<bool> m_LCMode{this,"LCMode",false,"Toggle whether we are in LC mode - false by default"};

  /** Toggle usage of calibration hit truth - false by default */
  Gaudi::Property<bool> m_useCalibHitTruth{this,"UseCalibHitTruth",false,"Toggle usage of calibration hit truth - false by default"};\

};

#endif
