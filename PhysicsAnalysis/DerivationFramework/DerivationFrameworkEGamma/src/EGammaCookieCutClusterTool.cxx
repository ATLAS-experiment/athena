/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Decorate egamma objects with cluster moments from a cookie-cut cluster

#include "DerivationFrameworkEGamma/EGammaCookieCutClusterTool.h"

#include "CaloEvent/CaloCellContainer.h"
#include "CaloUtils/CaloClusterStoreHelper.h"

#include "xAODEgamma/EgammaxAODHelpers.h"

namespace {
  constexpr float cellEtaSize = 0.1;
  constexpr float cellPhiSize = 0.1;
}

// Athena initialize
StatusCode
DerivationFramework::EGammaCookieCutClusterTool::initialize()
{
  ATH_MSG_VERBOSE("initialize() ...");

  ATH_CHECK(m_caloDetDescrMgrKey.initialize());

  ATH_MSG_DEBUG("Will store cookie cut clusters in this container "
                << m_outClusterContainerKey.key());
  ATH_CHECK(m_outClusterContainerKey.initialize());
  ATH_CHECK(m_outClusterContainerCellLinkKey.initialize());

  ATH_MSG_DEBUG("Using " << m_SGKey_electrons << " for electrons");
  ATH_CHECK(m_SGKey_electrons.initialize());

  m_nDecor = m_SGKey_electrons_decorations.size();
  ATH_MSG_DEBUG("Initialize nDecor = " << m_nDecor);
  ATH_CHECK(m_SGKey_electrons_decorations.initialize());
  if (msgLvl(MSG::DEBUG)) {
    ATH_MSG_DEBUG("Decorations for " << m_SGKey_electrons.key());
    for (const auto& s : m_SGKey_electrons_decorations)
      { ATH_MSG_DEBUG(s.key()); }
  }

  m_CookieCutPars.maxDelEta = m_maxDelEtaCells * cellEtaSize * 0.5;
  m_CookieCutPars.maxDelPhi = m_maxDelPhiCells * cellPhiSize * 0.5;
  m_CookieCutPars.maxDelR2  = m_maxDelR * m_maxDelR;
  m_CookieCutPars.fixCellWeights = m_fixCellWeights;

  // Retrieve cluster moment maker in case we cookie cut and want to recompute
  if (!m_clusterCorrectionTools.empty()) {
    ATH_CHECK(m_clusterCorrectionTools.retrieve());
    m_CookieCutPars.recomputeMoments = true;
  }

  return StatusCode::SUCCESS;
}

// The decoration itself
StatusCode
DerivationFramework::EGammaCookieCutClusterTool::addBranches(const EventContext& ctx) const
{

  // For debug
  static const std::vector<CaloSampling::CaloSample> s_sam
    { CaloSampling::PreSamplerB, CaloSampling::EMB1,
      CaloSampling::EMB2, CaloSampling::EMB3,
      CaloSampling::PreSamplerE, CaloSampling::EME1,
      CaloSampling::EME2, CaloSampling::EME3,
      CaloSampling::TileBar0, CaloSampling::TileBar1, CaloSampling::TileBar2,
      CaloSampling::TileExt0, CaloSampling::TileExt1, CaloSampling::TileExt2,
      CaloSampling::TileGap1, CaloSampling::TileGap2, CaloSampling::TileGap3,
      CaloSampling::HEC0, CaloSampling::HEC1,
      CaloSampling::HEC2, CaloSampling::HEC3,
      CaloSampling::FCAL0, CaloSampling::FCAL1, CaloSampling::FCAL2 };

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

  // Retrieve electron container
  SG::ReadHandle<xAOD::EgammaContainer> electronContainer(m_SGKey_electrons,
                                                          ctx);
  const std::size_t nF = electronContainer.ptr()->size();
  if (nF == 0) {
    return StatusCode::SUCCESS;
  }

  std::vector<SG::WriteDecorHandle<xAOD::EgammaContainer, float>> decoM;
  decoM.reserve(m_nDecor);
  for (int i = 0; i < m_nDecor - 1; i++) {
    decoM.emplace_back(m_SGKey_electrons_decorations[i], ctx);
  }

  if (msgLevel(MSG::DEBUG)) {
    std::call_once(m_Seen, [this,&decoM]() {
      for (int i = 0; i < m_nDecor - 1; i++) {
        ATH_MSG_DEBUG("Decor " << i << " out of " << m_nDecor
                      << " " << m_SGKey_electrons_decorations[i].key()
                      << " " << decoM[i].decorKey());
      }
    });
  }
  SG::WriteDecorHandle<xAOD::EgammaContainer,
                       ElementLink<xAOD::CaloClusterContainer>>
    decoEl(m_SGKey_electrons_decorations[m_nDecor-1],ctx);

  // Calorimeter description.
  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{
    m_caloDetDescrMgrKey, ctx
  };
  ATH_CHECK(caloDetDescrMgrHandle.isValid());
  const CaloDetDescrManager* calodetdescrmgr = *caloDetDescrMgrHandle;

  // Decorate electrons
  int iel = 0;
  std::vector<bool> success(nF,true);
  ATH_MSG_VERBOSE("Will analyze " << nF << " forward electrons");
  for (const auto* electron : *electronContainer.ptr()) {

    const xAOD::CaloCluster *cluster = electron->caloCluster();
    ATH_MSG_DEBUG("Electron " << electron->index() << " cluster"
                  << " pT = " << cluster->pt()
                  << " eta = " << cluster->eta());

    const DataLink<CaloCellContainer>& cellCont =
      cluster->getCellLinks()->getCellContainerLink();

    std::unique_ptr<xAOD::CaloCluster> newCluster =
      egammaClusterCookieCut::cookieCut(*cluster, *calodetdescrmgr,
                                        cellCont, m_CookieCutPars);

    if (!newCluster || newCluster->getCellLinks()->size() == 0) {
      ATH_MSG_DEBUG("No able to build a new cluster for electron "
                    << electron->index()
                    << " pT = " << electron->pt()
                    << " eta = " << electron->eta()
                    << " original cluster " << cluster->index()
                    << " pT = " << cluster->pt()
                    << " eta = " << cluster->eta()
                    << " has EME2 or FCAL0"
                    << " " << cluster->hasSampling(CaloSampling::EME2)
                    << " " << cluster->hasSampling(CaloSampling::FCAL0)
                    << " reason : "
                    << (newCluster ? " no cells in cluster" : " no cluster"));
      if (!newCluster) {
        ATH_MSG_VERBOSE("Energies in various samplings :");
        for (auto s : s_sam)
          ATH_MSG_VERBOSE("Sampling " << s << " E = " << cluster->eSample(s));
      }
      decoEl(*electron) = ElementLink<xAOD::CaloClusterContainer>();
      success[iel] = false;
    }

    if (success[iel]) {
      if (newCluster) {
        ATH_MSG_DEBUG("Cookie cut cluster"
                      << " pT = " << newCluster->pt()
                      << " eta = " << newCluster->eta());
      }
      outClusterContainer->push_back(std::move(newCluster));
      size_t index = outClusterContainer->size() - 1;
      ElementLink<xAOD::CaloClusterContainer>
        clusterLink(*outClusterContainer, index, ctx);
      // Now decorate the electron with the link to cookie cut cluster
      decoEl(*electron) = clusterLink;
    }

    // Eventually decorate the electron with the original moment
    if (m_storeOrigMom) {
      for (size_t i = 0; i < m_vecM.size(); i++) {
        int indexDecor = m_storeCookMom ? 2*i+1 : i;
        double m = 0.;
        bool gotM = cluster->retrieveMoment(
	     static_cast<xAOD::CaloCluster::MomentType>(m_vecM[i]), m);
        if (!gotM) {
          ATH_MSG_VERBOSE(m_vecMName[i]
                          << " does not exist for the original cluster");
        }
        decoM[indexDecor](*electron) = float(m);
        ATH_MSG_VERBOSE("Decorated electron " << electron->index()
                        << " with original moment "
                        << i << " " << m_vecMName[i]
                        << " = " << m << " (decoration index =" << indexDecor
                        << " key = " << decoM[indexDecor].decorKey() << ")");
      }
    }
    iel++;
  }

  if (!m_clusterCorrectionTools.empty()) {
    ToolHandleArray<CaloClusterCollectionProcessor>::const_iterator
      toolIt = m_clusterCorrectionTools.begin(),
      toolIt_e = m_clusterCorrectionTools.end();
    for (; toolIt != toolIt_e; ++toolIt) {
      ATH_CHECK((*toolIt)->execute(ctx, outClusterContainer.ptr()));
    }

    if (m_storeCookMom) {
      static const SG::AuxElement::Accessor<
        ElementLink< xAOD::CaloClusterContainer > >
        cookClusLinkAcc( "cookiecutClusterLink" );
      iel = 0;
      for (const auto* electron : *electronContainer.ptr()) {
        const xAOD::CaloCluster *cluster = nullptr;
        if (success[iel]) {
	  cluster = xAOD::EgammaHelpers::getCluster(electron);
        }
        if (success[iel] && cluster == nullptr) {
          ATH_MSG_WARNING("CookieCut cluster was build successfully, but"
                          "could not be retrieved");
        }

        for (size_t i = 0; i < m_vecM.size(); i++) {
          int indexDecor = m_storeOrigMom ? 2*i : i;
          double m = 0.;
          if (cluster) {
            bool gotM = cluster->retrieveMoment(
		 static_cast<xAOD::CaloCluster::MomentType>(m_vecM[i]), m);
            if (!gotM) {
              ATH_MSG_VERBOSE(m_vecMName[i]
                              << " does not exist for the new cluster");
            }
          }
          decoM[indexDecor](*electron) = float(m);
          ATH_MSG_VERBOSE("Decorated electron " << electron->index()
                          << " with new moment "
                          << i << " " << m_vecMName[i]
                          << " = " << m << " (decoration index = " << indexDecor
                          << " key = " << decoM[indexDecor].decorKey() << ")");
        }
	iel++;
      }
    }
  }

  CaloClusterStoreHelper::finalizeClusters(
                                           ctx,
                                           outClusterContainer,
                                           outClusterContainerCellLink);

  ATH_MSG_DEBUG("Built " << outClusterContainer.ptr()->size()
                << " cookie cut clusters");

  return StatusCode::SUCCESS;
}
