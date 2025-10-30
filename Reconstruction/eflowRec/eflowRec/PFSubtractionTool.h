/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFSUBTRACTIONTOOL_H
#define PFSUBTRACTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "eflowRec/eflowCaloObject.h"
#include "eflowRec/EtaPhiLUT.h"
#include "eflowRec/IPFSubtractionTool.h"
#include "eflowRec/PFData.h"
#include "eflowRec/PFEnergyPredictorTool.h"
#include "eflowRec/PFMatchPositions.h"
#include "eflowRec/PFTrackClusterMatchingTool.h"
#include "eflowRec/PFSimulateTruthShowerTool.h"
#include "eflowRec/PFSubtractionStatusSetter.h"
#include "eflowRec/PFSubtractionEnergyRatioCalculator.h"
#include "eflowRec/eflowSubtractor.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODTracking/TrackParticle.h"

class eflowCaloObjectContainer;
class eflowEEtaBinnedParameters;
class eflowRecClusterContainer;
class eflowRecTrackContainer;
class IEFlowCellEOverPTool;

class PFSubtractionTool : public extends<AthAlgTool, IPFSubtractionTool>
{

public:
  PFSubtractionTool(const std::string &type, const std::string &name, const IInterface *parent);
  ~PFSubtractionTool();

  StatusCode initialize();
  void execute(eflowCaloObjectContainer *theEflowCaloObjectContainer, eflowRecTrackContainer *recTrackContainer, eflowRecClusterContainer *recClusterContainer) const;
  StatusCode finalize();

private:  

  /** This matches ID tracks and CaloClusters, and then creates eflowCaloObjects */
  unsigned int matchAndCreateEflowCaloObj(PFData &data) const;

  void performSubtraction(const unsigned int& startingPoint,PFData &data) const;
  void performSubtraction(eflowCaloObject& thisEflowCaloObject) const;
  void performTruthSubtraction(PFData &data) const;
  void performTruthSubtraction(eflowCaloObject& thisEflowCaloObject) const;

  bool isEOverPFail(double expectedEnergy, double sigma, double clusterEnergy) const;

  bool canAnnihilate(double expectedEnergy, double sigma, double clusterEnergy) const;

  static std::string printTrack(const xAOD::TrackParticle* track);
  static std::string printCluster(const xAOD::CaloCluster* cluster);
  void printAllClusters(const eflowRecClusterContainer& recClusterContainer) const;

  void addSubtractedCells(eflowCaloObject& thisEflowCaloObject, const std::vector<std::pair<xAOD::CaloCluster *, bool> >& clusterList) const;

  /** Tool for getting e/p values and hadronic shower cell ordering principle parameters */
  ToolHandle<IEFlowCellEOverPTool> m_theEOverPTool{this, "eflowCellEOverPTool", "eflowCellEOverPTool", "Energy Flow E/P Values and Shower Parameters Tool"};

  std::unique_ptr<eflowEEtaBinnedParameters> m_binnedParameters;

  /** Track position provider to be used to preselect clusters */
  std::unique_ptr<PFMatch::TrackEtaPhiInFixedLayersProvider> m_trkpos;

  /** Default track-cluster matching tool */
  ToolHandle<PFTrackClusterMatchingTool> m_theMatchingTool{this, "PFTrackClusterMatchingTool", "PFTrackClusterMatchingTool/CalObjBldMatchingTool", "The track-cluster matching tool"};

  /* Track-cluster matching tools for calculating the pull */
  ToolHandle<PFTrackClusterMatchingTool> m_theMatchingToolForPull_015{this, "PFTrackClusterMatchingTool_015", "PFTrackClusterMatchingTool/PFPullMatchingTool_015", "The 0.15 track-cluster matching tool to calculate the pull"};
  ToolHandle<PFTrackClusterMatchingTool> m_theMatchingToolForPull_02{this, "PFTrackClusterMatchingTool_02", "PFTrackClusterMatchingTool/PFPullMatchingTool_02", "The 0.2 track-cluster matching tool to calculate the pull"};

  /** Toggle whether we are recovering split showers or not */
  Gaudi::Property<bool> m_recoverSplitShowers{this,"RecoverSplitShowers",false,"Toggle whether we are recovering split showers or not"};

  /** Number of clusters to match to each track if not doing recover split shower subtraction */
  Gaudi::Property<int> m_nClusterMatchesToUse{this, "nClusterMatchesToUse", 1, "Number of clusters to match to each track"};

  /** Toggle whether we have the HLLHC setup */
  Gaudi::Property<bool> m_isHLLHC{this, "isHLLHC", false, "Toggle whether we have the HLLHC setup"};

  /** Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed */
  Gaudi::Property<bool> m_calcEOverP{this, "CalcEOverP", false, "Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed"};

  /** Parameter that controls whether a track, in a track-cluster system, will be processed by the split shower recovery algorithm */
  Gaudi::Property<double> m_consistencySigmaCut{this, "ConsistencySigmaCut", 1.0, "Parameter that controls whether a track, in a track-cluster system, will be processed by the split shower recovery algorithm"};

  /** Parameter that controls whether to use retain remaining calorimeter energy in track-cluster system, after charged shower subtraction */
  Gaudi::Property<double> m_subtractionSigmaCut{this, "SubtractionSigmaCut", 1.5, "Parameter that controls whether to use retain remaining calorimeter energy in track-cluster system, after charged shower subtraction"};

  /** Toggle whether to decorate eflowRecTrack with addutional data for Combined Performance studies */
  Gaudi::Property<bool> m_addCPData{this,"addCPData",false,"Toggle whether to decorate FlowElements with addutional data for Combined Performance studies "};

  //Helpers
  PFSubtractionStatusSetter m_pfSubtractionStatusSetter{};
  PFSubtractionEnergyRatioCalculator m_pfSubtractionEnergyRatioCalculator{};
  eflowSubtract::Subtractor m_subtractor{};

  /** Tool for getting predictiing the energy using an ONNX model */
  ToolHandle<PFEnergyPredictorTool> m_NNEnergyPredictorTool{this, "NNEnergyPredictorTool", "","Tool for getting predictiing the energy using an ONNX model "};
  
  /** Toggle whether we use the neural net energy */
  Gaudi::Property<bool> m_useNNEnergy{this, "useNNEnergy", false, "Toggle whether we use the neural net energy"};

  //Only turn this on for MC samples without pileup that have calibration hits avaialble
  /** Toggle whether to cheat and use truth information for track-cluster matching - only for performance studies! */
  Gaudi::Property<bool> m_useTruthMatching{this, "useTrackClusterTruthMatching", false, "Toggle whether to cheat and use truth information for track-cluster matching"};

  /** Toggle whether we use truth information for the charged shower subtraction or not */
  Gaudi::Property<bool> m_useTruthForChargedShowerSubtraction{this, "useTruthForChargedShowerSubtraction", false, "Toggle whether we use truth information for the charged shower subtraction or not"};

  /**Toggle whether we fully remove a cell with a truth deposit or reweight it based on truth contribution */
  Gaudi::Property<bool> m_useFullCellTruthSubtraction{this,"useFullCellTruthSubtraction",true,"Toggle whether we fully remove a cell with a truth deposit or reweight it based on truth contribution"};

  ToolHandle<PFSimulateTruthShowerTool> m_theTruthShowerSimulator{this, "PFSimulateTruthShowerTool", "", "The truth shower simulator"};

  /** Read handle key to decorate CaloCluster with threeN leading truth particle uniqueID and energy */
  SG::ReadDecorHandleKey<xAOD::CaloClusterContainer> m_caloClusterReadDecorHandleKeyNLeadingTruthParticles{this,"CaloClusterReadDecorHandleKey_NLeadingTruthParticles",""};

  /** Further discussion about why this flag exists can be found in https://its.cern.ch/jira/browse/ATLJETMET-1692
   and https://indico.cern.ch/event/1388633/contributions/5837876/attachments/2809591/4903439/PFlow_EOverP_Feb2024.pdf
   The Jira report discusses assorted problems with the treatment of energy bin indexes for the lookup of e/p values. In order to not change
   the behaviour of produciton code a legacy option is introduced to allow us to fix the problem in future iterations of e/p derivations.s
  */
  Gaudi::Property<bool> m_useLegacyEBinIndex{this, "useLegacyEBinIndex", true, "Toggle whether we use the legacy energy bin index"};

};

#endif
