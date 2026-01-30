/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaForwardBuilder.h"
#include "egammaInterfaces/IegammaBaseTool.h"
#include "egammaCaloUtils/CookieCutterHelpers.h"
#include "egammaUtils/EMFourMomBuilder.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/Electron.h"

#include "EgammaAnalysisInterfaces/IAsgForwardElectronIsEMSelector.h"
#include "PATCore/AcceptData.h"

#include <algorithm>
#include <cmath>

namespace {
  constexpr float cellEtaSize = 0.1;
  constexpr float cellPhiSize = 0.1;
}

egammaForwardBuilder::egammaForwardBuilder(const std::string& name,
                                           ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode egammaForwardBuilder::initialize()
{
  m_CookieCutPars.maxDelEta = m_maxDelEtaCells * cellEtaSize * 0.5;
  m_CookieCutPars.maxDelPhi = m_maxDelPhiCells * cellPhiSize * 0.5;
  m_CookieCutPars.maxDelR2  = m_maxDelR * m_maxDelR; // Square now to avoid a slow sqrt later.

  // The data handle keys.
  ATH_CHECK(m_topoClusterKey.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());
  ATH_CHECK(m_electronOutputKey.initialize());
  ATH_CHECK(m_outClusterContainerKey.initialize());
  if (m_outClusterContainerCellLinkKey.key().empty()) {
    m_outClusterContainerCellLinkKey = m_outClusterContainerKey.key() + "_links";
  }
  ATH_CHECK(m_outClusterContainerCellLinkKey.initialize());

  // Retrieve object quality tool.
  if (!m_objectQualityTool.empty()) {
    ATH_CHECK(m_objectQualityTool.retrieve());
  }

  else {
    m_objectQualityTool.disable();
  }

  ATH_CHECK(m_forwardElectronIsEMSelectors.retrieve());

  if (
    m_forwardElectronIsEMSelectors.size() !=
    m_forwardElectronIsEMSelectorResultNames.size()
  ) {
    ATH_MSG_ERROR(
      "Number of selectors doesn't match number of given fwd-electron selector names"
    );

    return StatusCode::FAILURE;
  }

  // Retrieve track match builder.
  ATH_CHECK(RetrieveEMTrackMatchBuilder());

  // Retrive MVA energy calibration
  if (m_doEnergyCal) {
    ATH_CHECK(m_pVtxKey.initialize());
    ATH_CHECK(m_eiKey.initialize());
    ATH_CHECK(m_MVACalibSvc.retrieve());
  }

  ATH_MSG_DEBUG("Initialization completed successfully");

  return StatusCode::SUCCESS;
}

StatusCode egammaForwardBuilder::finalize()
{

  ATH_MSG_INFO("====> Forward Egamma Statistics =============");
  ATH_MSG_INFO(" All Clusters " << m_AllClusters);
  ATH_MSG_INFO(" Matched Clusters " << m_MatchedClusters);
  ATH_MSG_INFO("=============================================");

  return StatusCode::SUCCESS;
}

StatusCode egammaForwardBuilder::execute(const EventContext& ctx) const
{
  // Create an egamma container and register it.
  SG::WriteHandle<xAOD::ElectronContainer> xaodFrwd(m_electronOutputKey, ctx);
  ATH_CHECK(xaodFrwd.record(
    std::make_unique<xAOD::ElectronContainer>(),
    std::make_unique<xAOD::ElectronAuxContainer>())
  );

  // Create the relevant cluster output and register it.
  SG::WriteHandle<xAOD::CaloClusterContainer> outClusterContainer(
    m_outClusterContainerKey,
    ctx
  );

  ATH_CHECK(CaloClusterStoreHelper::AddContainerWriteHandle(outClusterContainer));
  SG::WriteHandle<CaloClusterCellLinkContainer> outClusterContainerCellLink(
    m_outClusterContainerCellLinkKey,
    ctx
  );

  ATH_CHECK(outClusterContainerCellLink.record(
    std::make_unique<CaloClusterCellLinkContainer>())
  );

  // Topo cluster container.
  SG::ReadHandle<xAOD::CaloClusterContainer> inputClusters(m_topoClusterKey, ctx);

  // Check is only used for serial running, remove when MT scheduler used.
  ATH_CHECK(inputClusters.isValid());

  // Calorimeter description.
  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{
    m_caloDetDescrMgrKey, ctx
  };
  ATH_CHECK(caloDetDescrMgrHandle.isValid());
  const CaloDetDescrManager* calodetdescrmgr = *caloDetDescrMgrHandle;

  static const SG::AuxElement::Accessor<
    std::vector<ElementLink<xAOD::CaloClusterContainer>>
  > caloClusterLinks("constituentClusterLinks");

  // Prepare to create clusters.
  EgammaRecContainer egammaRecsFwd;
  size_t origClusterIndex = 0;

  egammaMVACalib::GlobalEventInfo gei;
  if (m_doEnergyCal) {
    std::pair<float,float> nPVmu = this->getnPVmu(ctx);
    gei.nPV = nPVmu.first;
    gei.acmu = nPVmu.second;
    ATH_MSG_DEBUG("Retrieved nPV = " << gei.nPV << " and mu = " << gei.acmu);
  }

  // Loop over input cluster container and create egRecs to store the electrons.
  ATH_MSG_VERBOSE("Will run on " << inputClusters->size() << " input clusters, doing cookie cut ? " << m_doCookieCutting);
  for (const xAOD::CaloCluster* cluster : *inputClusters) {

    // Create links back to the original clusters.
    std::vector<ElementLink<xAOD::CaloClusterContainer>> constituentLinks;

    // The constituent links should contain a CaloCal cluster. When not running
    // in ITk mode this is the default for the forward clusters used by egamma
    // so no sister link is needed to get the CaloCal. When running in ITk mode
    // the clusters used are CaloTopoClusters so need to access the sister
    // cluster to maintain consistency.
    if (m_doTrackMatching) {
      ElementLink<xAOD::CaloClusterContainer> sisterCluster =
        cluster->getSisterClusterLink();

      if (sisterCluster) {
        constituentLinks.push_back(sisterCluster);
      } else {
        ATH_MSG_WARNING("No sister Link available");
      }
    } else {
      constituentLinks.emplace_back(*inputClusters, origClusterIndex, ctx);
    }

    const DataLink<CaloCellContainer>& cellCont =
      cluster->getCellLinks()->getCellContainerLink();

    // Create the new cluster.
    std::unique_ptr<xAOD::CaloCluster> newCluster =
      m_doCookieCutting ?
      egammaClusterCookieCut::cookieCut(*cluster, *calodetdescrmgr,
					cellCont, m_CookieCutPars) :
        std::make_unique<xAOD::CaloCluster>(*cluster);

    if (!newCluster || newCluster->size() == 0) {
      ATH_MSG_DEBUG("Could not build a new cluster, or has 0 cell");
      continue;
    }

    if (m_doEnergyCal &&
	m_MVACalibSvc->execute(*newCluster, xAOD::EgammaParameters::forwardelectron, gei).isFailure())
      { ATH_MSG_ERROR("Problem executing MVA cluster tool for fwd electron"); }

    caloClusterLinks(*newCluster) = constituentLinks;
    outClusterContainer->push_back(std::move(newCluster));

    size_t index = outClusterContainer->size() - 1;
    const ElementLink<xAOD::CaloClusterContainer> clusterLink(*outClusterContainer, index, ctx);
    const std::vector<ElementLink<xAOD::CaloClusterContainer>> clusterLinkVector{clusterLink};

    // Now create the egamma Rec
    egammaRecsFwd.push_back(std::make_unique<egammaRec>(clusterLinkVector));

    ++origClusterIndex;
  }

  // Add track-cluster matching information if requested.
  if (m_doTrackMatching) {
    ATH_CHECK(m_trackMatchBuilder->executeRec(ctx, &egammaRecsFwd));
  }

  auto buff_AllClusters = m_AllClusters.buffer();
  auto buff_MatchedClusters = m_MatchedClusters.buffer();

  //Loop over the egamma Rec creating electrons
  for (const egammaRec* egRec : egammaRecsFwd) {
    if (!egRec) {
      return StatusCode::FAILURE;
    }

    ++buff_AllClusters;

    if (m_doTrackMatching && egRec->getNumberOfTrackParticles() == 0) {
      // Later we may want to use these for so called forward photons.
      ATH_MSG_DEBUG("EgammaRec without track particle");
      continue;
    }

    //common part
    xAOD::Electron* el = xaodFrwd->push_back(std::make_unique<xAOD::Electron>());
    el->setAuthor(xAOD::EgammaParameters::AuthorFwdElectron);
    el->setCaloClusterLinks(egRec->caloClusterElementLinks());

    // from here one, we need both track matching and
    // having tracks .
    if (m_doTrackMatching) {

      ++buff_MatchedClusters;
      el->setTrackParticleLinks(egRec->trackParticleElementLinks());

      const xAOD::TrackParticle* trackParticle = el->trackParticle();
      if (trackParticle) {
        el->setCharge(trackParticle->charge());
      } else {
        ATH_MSG_WARNING("Forward electron without track particle, whereas"
                        " corresponding egammaRec has at least one");
      }

      // Set DeltaEta, DeltaPhi, DeltaPhiRescaled.
      el->setTrackCaloMatchValues(
        egRec->deltaEta(),
        egRec->deltaPhi(),
        egRec->deltaPhiRescaled(),
        egRec->deltaPhiLast()
      );
    }

    EMFourMomBuilder::calculate(*el);
    ATH_CHECK(ExecObjectQualityTool(ctx, el));

    // Apply the Forward Electron selectors.
    for (size_t i = 0; i < m_forwardElectronIsEMSelectors.size(); ++i) {
      const auto selector = m_forwardElectronIsEMSelectors[i];
      const auto name = m_forwardElectronIsEMSelectorResultNames[i];

      // Save the bool result.
      const asg::AcceptData accept = selector->accept(ctx, el);
      el->setPassSelection(static_cast<bool>(accept), name);

      // Save the isem.
      el->setSelectionisEM(accept.getCutResultInverted(), "isEM" + name);
    }

  }//end of loop over egammaRecs

  CaloClusterStoreHelper::finalizeClusters(
    ctx,
    outClusterContainer,
    outClusterContainerCellLink);

  return StatusCode::SUCCESS;
}

StatusCode
egammaForwardBuilder::ExecObjectQualityTool(
  const EventContext& ctx,
  xAOD::Egamma* eg
) const {
  // Protection in case tool is not available return success as algorithm can run without it.
  if (!m_objectQualityTool.isEnabled()) { return StatusCode::SUCCESS; }

  return m_objectQualityTool->execute(ctx,*eg);
}

StatusCode
egammaForwardBuilder::RetrieveEMTrackMatchBuilder()
{
  if (!m_doTrackMatching) {
    m_trackMatchBuilder.disable();
    return StatusCode::SUCCESS;
  }

  if (m_trackMatchBuilder.empty()) {
    ATH_MSG_ERROR("EMTrackMatchBuilder is empty, but track matching is enabled");
    return StatusCode::FAILURE;
  }

  if (m_trackMatchBuilder.retrieve().isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve " << m_trackMatchBuilder);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

std::pair<unsigned int, float> egammaForwardBuilder::getnPVmu(
  const EventContext& ctx) const {
  unsigned int npv(0);
  float mu(0);

  SG::ReadHandle<xAOD::VertexContainer> vtxCont(m_pVtxKey, ctx);
  if (!vtxCont.isValid()) {
    ATH_MSG_WARNING("Cannot find " << m_pVtxKey.key()
            << " container, returning nPV = 0");
  } else {
    for (const auto *vtx : *vtxCont) {
      if (vtx->vertexType() == xAOD::VxType::PriVtx ||
	  vtx->vertexType() == xAOD::VxType::PileUp) { ++npv; }
    }
  }

  SG::ReadHandle<xAOD::EventInfo> eiCont(m_eiKey, ctx);
  if (!eiCont.isValid()) {
    ATH_MSG_WARNING("Cannot find " << m_eiKey.key() << " returning mu = 0");
  } else {
    mu = eiCont.get()->actualInteractionsPerCrossing();
  }

  return std::make_pair(npv,mu);
}
