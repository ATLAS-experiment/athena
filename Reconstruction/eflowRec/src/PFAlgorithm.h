/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef EFLOWREC_PFALGORITHM_H
#define EFLOWREC_PFALGORITHM_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/Monitored.h"

#include "eflowCaloObject.h"
#include "eflowRecTrack.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "IPFClusterSelectorTool.h"
#include "IPFBaseTool.h"
#include "IPFSubtractionTool.h"
#include "IPFUnifiedBaseTool.h"

#include "PFData.h"


class eflowRecClusterContainer;

class PFAlgorithm : public AthReentrantAlgorithm {

public:
  PFAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
  ~PFAlgorithm() {};

  StatusCode initialize() override ;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;

private:
  /** ToolHandle for the PFClusterSelectorTool which creates the set of
   * eflowRecCluster to be used */
  ToolHandle<IPFClusterSelectorTool> m_IPFClusterSelectorTool{
    this,
    "PFClusterSelectorTool",
    "PFClusterSelectorTool",
    "ToolHandle for the PFClusterSelectorTool which creates the set of "
    "eflowRecCluster to be used"
  };

  /** List of IPFSubtractionTool, which will be executed by this algorithm.
      Legacy chain, configured by the HLT only - see m_useUnified */
  ToolHandleArray<IPFSubtractionTool> m_IPFSubtractionTools{this, "SubtractionToolList", {}, "List of Private Subtraction IPFSubtractionTools"};

  /** List of PFBaseAlgTool, which will be executed by this algorithm.
      Legacy chain, configured by the HLT only - see m_useUnified */
  ToolHandleArray<IPFBaseTool> m_IPFBaseTools{this, "BaseToolList", {}, "List of Private IPFBaseTools"};

  /** List of IPFUnifiedBaseTool, which will be executed by this algorithm */
  ToolHandleArray<IPFUnifiedBaseTool> m_IPFUnifiedBaseTools{this, "UnifiedBaseTools", {}, "List of Private IPFUnifiedBaseTools"};

  /** ReadHandleKey for the eflowRecTrackContainer to be read in */
  SG::ReadHandleKey<eflowRecTrackContainer> m_eflowRecTracksReadHandleKey{
    this,
    "eflowRecTracksInputName",
    "eflowRecTracks",
    "ReadHandleKey for the eflowRecTrackContainer to be read in"
  };

  /** WriteHandleKey for the eflowRecClusterContainer to write out */
  SG::WriteHandleKey<eflowRecClusterContainer> m_eflowRecClustersWriteHandleKey{
    this,
    "eflowRecClustersOutputName",
    "eflowRecClusters",
    "WriteHandleKey for the eflowRecClusterContainer to write out"
  };

  /** WriteHandleKey for CaloClusterContainer to be written out */
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_caloClustersWriteHandleKey{
    this,
    "PFCaloClustersOutputName",
    "PFCaloCluster",
    "WriteHandleKey for CaloClusterContainer to be written out"
  };

  /** WriteHandleKey for eflowCaloObjectContainer to be written out */
  SG::WriteHandleKey<eflowCaloObjectContainer> m_eflowCaloObjectsWriteHandleKey{
    this,
    "eflowCaloObjectsOutputName",
    "eflowCaloObjects",
    "WriteHandleKey for eflowCaloObjectContainer to be written out"
  };

  /** Online monitoring tool for recording histograms of the alg in action */
  ToolHandle<GenericMonitoringTool> m_monTool{ this,
                                               "MonTool",
                                               "",
                                               "Monitoring tool" };

  /** Funciton to print out list of tools if in VERBOSE mode */
  void printTools();

  /** Selects the unified tool chain (m_IPFUnifiedBaseTools) over the legacy one
      (m_IPFSubtractionTools + m_IPFBaseTools). Offline reconstruction always
      sets this true; only the HLT configuration (PFHLTConfig) still leaves it
      false. Once the HLT is ported this property and the two legacy tool lists
      above can be removed, along with PFSubtractionTool and
      PFMomentCalculatorTool. */
  Gaudi::Property<bool> m_useUnified{this, "useUnified", false, "Toggle to use standard PFA or unified PFA setup"};

};
#endif
