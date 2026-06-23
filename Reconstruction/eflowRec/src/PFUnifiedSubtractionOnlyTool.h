/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFUNIFIEDSUBTRACTIONONLYTOOL_H
#define PFUNIFIEDSUBTRACTIONONLYTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "IPFUnifiedBaseTool.h"
#include "PFEnergyPredictorTool.h"
#include "PFMatchPositions.h"
#include "PFSubtractionStatusSetter.h"
#include "PFSubtractionEnergyRatioCalculator.h"
#include "eflowSubtractor.h"
#include "xAODCaloEvent/CaloClusterFwd.h"


class eflowCaloObject;
class eflowCaloObjectContainer;
class eflowEEtaBinnedParameters;
class IEFlowCellEOverPTool;
struct PFData;


class PFUnifiedSubtractionOnlyTool : public extends<AthAlgTool, IPFUnifiedBaseTool>
{

public:
  PFUnifiedSubtractionOnlyTool(const std::string &type, const std::string &name, const IInterface *parent);
  ~PFUnifiedSubtractionOnlyTool();

  virtual StatusCode initialize() override;
  virtual StatusCode processPFlowData(
    const EventContext& ctx,
    PFData &thePFData
  ) const override;

protected:

  /** This matches ID tracks and CaloClusters, and then creates eflowCaloObjects */
  unsigned int matchAndCreateEflowCaloObj(PFData &data) const;

  virtual void performSubtraction(const EventContext& ctx, const unsigned int& startingPoint, const unsigned int& nCaloObj, PFData &data ) const;
  virtual void performSubtraction(eflowCaloObject& thisEflowCaloObject) const;
  void simulateShowers(const EventContext& ctx, eflowCaloObjectContainer& eflowCaloObjects, const unsigned int& nCaloObj) const;

  bool isEOverPFail(double expectedEnergy, double sigma, double clusterEnergy) const;

  bool canAnnihilate(double expectedEnergy, double sigma, double clusterEnergy) const;

  void addSubtractedCells(eflowCaloObject& thisEflowCaloObject, const std::vector<std::pair<xAOD::CaloCluster *, bool> >& clusterList) const;

  /** Tool for getting e/p values and hadronic shower cell ordering principle parameters */
  ToolHandle<IEFlowCellEOverPTool> m_theEOverPTool{this, "eflowCellEOverPTool", "eflowCellEOverPTool", "Energy Flow E/P Values and Shower Parameters Tool"};

  std::unique_ptr<eflowEEtaBinnedParameters> m_binnedParameters;

  /** Track position provider to be used to preselect clusters */
  std::unique_ptr<PFMatch::TrackEtaPhiInFixedLayersProvider> m_trkpos;

  /** Toggle whether we are recovering split showers or not */
  Gaudi::Property<bool> m_recoverSplitShowers{this,"RecoverSplitShowers",false,"Toggle whether we are recovering split showers or not"};

  /** Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed */
  Gaudi::Property<bool> m_calcEOverP{this, "CalcEOverP", false, "Toggle EOverP algorithm mode, whereby no charged shower subtraction is performed"};

  /** Parameter that controls whether a track, in a track-cluster system, will be processed by the split shower recovery algorithm */
  Gaudi::Property<double> m_consistencySigmaCut{this, "ConsistencySigmaCut", 1.0, "Parameter that controls whether a track, in a track-cluster system, will be processed by the split shower recovery algorithm"};

  /** Parameter that controls whether to use retain remaining calorimeter energy in track-cluster system, after charged shower subtraction */
  Gaudi::Property<double> m_subtractionSigmaCut{this, "SubtractionSigmaCut", 1.5, "Parameter that controls whether to use retain remaining calorimeter energy in track-cluster system, after charged shower subtraction"};

  /** Toggle whether we have the HLLHC setup */
  Gaudi::Property<bool> m_isHLLHC{this, "isHLLHC", false, "Toggle whether we have the HLLHC setup"};

  /** Toggle whether to decorate eflowRecTrack with additional data for Combined Performance studies */
  Gaudi::Property<bool> m_addCPData{this,"addCPData",false,"Toggle whether to decorate FlowElements with additional data for Combined Performance studies "};

  //Helpers
  PFSubtractionStatusSetter m_pfSubtractionStatusSetter{};
  PFSubtractionEnergyRatioCalculator m_pfSubtractionEnergyRatioCalculator{};
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
