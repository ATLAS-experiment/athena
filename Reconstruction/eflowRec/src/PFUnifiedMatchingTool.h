/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFUNIFIEDMATCHINGTOOL_H
#define PFUNIFIEDMATCHINGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "IPFUnifiedBaseTool.h"
#include "PFEnergyPredictorTool.h"
#include "PFMatchPositions.h"
#include "PFTrackClusterMatchingTool.h"
#include "eflowSubtractor.h"
#include "xAODCaloEvent/CaloClusterFwd.h"
#include "xAODTracking/TrackParticleFwd.h"

class eflowEEtaBinnedParameters;
class eflowRecCluster;
class IEFlowCellEOverPTool;
struct PFData;


class PFUnifiedMatchingTool : public extends<AthAlgTool, IPFUnifiedBaseTool>
{

public:
  PFUnifiedMatchingTool(const std::string &type, const std::string &name, const IInterface *parent);
  ~PFUnifiedMatchingTool();

  virtual StatusCode initialize() override;
  virtual StatusCode processPFlowData(
    const EventContext& ctx,
    PFData &thePFData
  ) const override;

protected:  

  /** This matches ID tracks and CaloClusters, and then creates eflowCaloObjects */
  virtual unsigned int matchAndCreateEflowCaloObj(const EventContext& ctx, PFData &data) const;

  static std::string printTrack(const xAOD::TrackParticle* track);
  static std::string printCluster(const xAOD::CaloCluster* cluster);
  void printAllClusters(const std::vector<eflowRecCluster *>& recClusterVector) const;

  /** Tool for getting e/p values and hadronic shower cell ordering principle parameters */
  ToolHandle<IEFlowCellEOverPTool> m_theEOverPTool{this, "eflowCellEOverPTool", "eflowCellEOverPTool", "Energy Flow E/P Values and Shower Parameters Tool"};

  std::unique_ptr<eflowEEtaBinnedParameters> m_binnedParameters;

  /** Track position provider to be used to preselect clusters */
  std::unique_ptr<PFMatch::TrackEtaPhiInFixedLayersProvider> m_trkpos;

  /** Default track-cluster matching tool */
  ToolHandle<PFTrackClusterMatchingTool> m_theMatchingTool{this, "PFTrackClusterMatchingTool", "PFTrackClusterMatchingTool/CalObjBldMatchingTool", "The track-cluster matching tool"};

  /* Track-cluster matching tools for calculating the pull */
  ToolHandle<PFTrackClusterMatchingTool> m_theMatchingToolForPull_02{this, "PFTrackClusterMatchingTool_02", "PFTrackClusterMatchingTool/PFPullMatchingTool_02", "The 0.2 track-cluster matching tool to calculate the pull"};

  /** Toggle whether we are recovering split showers or not */
  Gaudi::Property<bool> m_recoverSplitShowers{this,"RecoverSplitShowers",false,"Toggle whether we are recovering split showers or not"};

  /** Number of clusters to match to each track if not doing recover split shower subtraction */
  Gaudi::Property<int> m_nClusterMatchesToUse{this, "nClusterMatchesToUse", 1, "Number of clusters to match to each track"};

  /** Toggle whether to decorate eflowRecTrack with additional data for Combined Performance studies */
  Gaudi::Property<bool> m_addCPData{this,"addCPData",false,"Toggle whether to decorate FlowElements with additional data for Combined Performance studies "};

  //Helpers
  eflowSubtract::Subtractor m_subtractor{};

  /** Tool for getting predictiing the energy using an ONNX model */
  ToolHandle<PFEnergyPredictorTool> m_NNEnergyPredictorTool{this, "NNEnergyPredictorTool", "","Tool for getting predictiing the energy using an ONNX model "};
  
  /** Toggle whether we use the neural net energy */
  Gaudi::Property<bool> m_useNNEnergy{this, "useNNEnergy", false, "Toggle whether we use the neural net energy"};

  /** Further discussion about why this flag exists can be found in https://its.cern.ch/jira/browse/ATLJETMET-1692
   and https://indico.cern.ch/event/1388633/contributions/5837876/attachments/2809591/4903439/PFlow_EOverP_Feb2024.pdf
   The Jira report discusses assorted problems with the treatment of energy bin indexes for the lookup of e/p values. In order to not change
   the behaviour of produciton code a legacy option is introduced to allow us to fix the problem in future iterations of e/p derivations.s
  */
  Gaudi::Property<bool> m_useLegacyEBinIndex{this, "useLegacyEBinIndex", true, "Toggle whether we use the legacy energy bin index"};

};

#endif
