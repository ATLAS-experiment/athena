/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_EGAMMAFORWARDBUILDER_H
#define EGAMMAALGS_EGAMMAFORWARDBUILDER_H

/**
  @class egammaForwardBuilder
          Algorithm which makes a egammaObjectCollection for forward electrons.
          egammaForwardBuilder, is dedicated to the reconstruction and 
          identification of electrons in the forward region of ATLAS 
          (2.5<|eta|<4.9). In contrast to the softe and egamma builders the 
          algorithm can use only the information from the calorimeters, as the 
          tracking system is limited to |eta|<2.5, and the topological 
          clusters (instead of SW clusters). The pre-selection and ID are 
          done in the same algorithm. The variables used to discriminant 
          between electron and hadrons are defined as the topo cluster moments 
          or combination of them. This is done separately in two eta bins: 
          the EMEC IW and the FCal using a cut based technic. 
          The forward electron AUTHOR is 8. 
*/

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/EventContext.h"

#include "EventKernel/IParticle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/ElectronContainer.h"

#include "egammaInterfaces/IegammaOQFlagsBuilder.h" 
#include "egammaInterfaces/IegammaBaseTool.h"
#include "egammaInterfaces/IEMFourMomBuilder.h"
#include "EgammaAnalysisInterfaces/IAsgForwardElectronIsEMSelector.h"
#include "egammaInterfaces/IEMTrackMatchBuilder.h"

#include <string>

#include <Gaudi/Accumulators.h>

class egammaForwardBuilder : public AthReentrantAlgorithm
{
public:
  /** @brief Constructor. */
  egammaForwardBuilder(const std::string& name, ISvcLocator* pSvcLocator);

  /** @brief Destructor. */
  ~egammaForwardBuilder();

  /** @brief Initialize method. */
  virtual StatusCode initialize() override final;

  /** @brief Finalize method. */
  virtual StatusCode finalize() override final;

  /** @brief Execute method. */
  virtual StatusCode execute(const EventContext& ctx) const override final;

private:
  StatusCode RetrieveEMTrackMatchBuilder();
  StatusCode ExecObjectQualityTool(const EventContext& ctx, xAOD::Egamma* eg) const;

  /** @brief Convinience wrapper to set track match values in all samplings. */
  void setAllTrackCaloMatchValues(
    xAOD::Electron *el,
    const std::array<xAOD::EgammaParameters::TrackCaloMatchType, 4> &match_parameters,
    const std::array<double, 4> &match_values
  ) const;

  /** @brief Remove cells that are too far from the center of mass. */
  void cookieCut(
    xAOD::CaloCluster& cluster,
    const CaloDetDescrManager& mgr
  ) const;

  /** @brief Tool to perform object quality. */
  ToolHandle<IegammaOQFlagsBuilder> m_objectQualityTool{
    this,
    "ObjectQualityTool",
    "",
    "Name of the object quality tool (empty tool name ignored)"
  };

  /** @brief Tool to perform the 4-mom computation. */
  ToolHandle<IEMFourMomBuilder> m_fourMomBuilder{
    this,
    "FourMomBuilderTool",
    "EMFourMomBuilder",
    "Handle of 4-mom Builder"
  };

  /** @brief Tool to perform track-cluster matching. */
  ToolHandle<IEMTrackMatchBuilder> m_trackMatchBuilder{
    this,
    "TrackMatchBuilderTool",
    "EMTrackMatchBuilder",
    "Tool that matches tracks to egammaRecs (Fwd)"
  };

  /** @brief Input topo cluster type. */
  SG::ReadHandleKey<xAOD::CaloClusterContainer> m_topoClusterKey{
    this,
    "TopoClusterName",
    "",
    "Name of the input cluster collection"
  };

  /** @brief Calorimeter description. */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloDetDescrMgrKey {
    this,
    "CaloDetDescrManager",
    "CaloDetDescrManager",
    "SG Key for CaloDetDescrManager in the Condition Store"
  };

  /** @brief Output electron container. */
  SG::WriteHandleKey<xAOD::ElectronContainer> m_electronOutputKey{
    this,
    "ElectronOutputName",
    "",
    "Name of Electron Container to be created"
  };

  /** @brief Output cluster container. */
  SG::WriteHandleKey<xAOD::CaloClusterContainer> m_outClusterContainerKey{
    this,
    "ClusterContainerName",
    ""
    "Name of the output EM cluster container"
  };

  /** @brief Output cluster container cell links: name taken from containter name. */
  SG::WriteHandleKey<CaloClusterCellLinkContainer> m_outClusterContainerCellLinkKey{
    this,
    "ClusterContainerLinks", "",
    "Key of the output cluster container cell links; Taken from associated container"};

  /** @brief Private member flag to do the track matching. */
  Gaudi::Property<bool> m_doTrackMatching { 
    this,
    "doTrackMatching",
    false,
    "Boolean to do track matching"
  };

  /** @brief Private member flag to do cookie cutting. */
  Gaudi::Property<bool> m_doCookieCutting { 
    this,
    "doCookieCutting",
    false,
    "Boolean to do cookie cutting"
  };

  /** @brief Size of maximum search window in eta. */
  Gaudi::Property<int> m_maxDelEtaCells{
    this,
    "MaxWindowDelEtaCells",
    3,
    "Size of maximum search window in eta"
  };

  /** @brief Size of maximum search window in phi. */
  Gaudi::Property<int> m_maxDelPhiCells{
    this,
    "MaxWindowDelPhiCells",
    3,
    "Size of maximum search window in phi"
  };

  /** @brief Size of cone to cookie cut on FCal. */
  Gaudi::Property<float> m_maxDelR{
    this,
    "MaxWindowDelR",
    0.3,
    "Cone size to collect cells around hottest-cell FCAL"
  };

  mutable Gaudi::Accumulators::Counter<> m_AllClusters {};
  mutable Gaudi::Accumulators::Counter<> m_MatchedClusters {};

  float m_maxDelEta {};
  float m_maxDelPhi {};
  float m_maxDelR2 {};

protected:
  /** Handle to the selectors. */
  ToolHandleArray<IAsgForwardElectronIsEMSelector> m_forwardElectronIsEMSelectors {
    this,
    "forwardelectronIsEMselectors", 
    {}, 
    "The selectors that we need to apply to the FwdElectron object"
  };

  Gaudi::Property<std::vector<std::string>> m_forwardElectronIsEMSelectorResultNames {
    this,
    "forwardelectronIsEMselectorResultNames", 
    {},
    "The selector result names"
  };
};
#endif

