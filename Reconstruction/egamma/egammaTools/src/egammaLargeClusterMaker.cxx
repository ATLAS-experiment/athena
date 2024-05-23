/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaLargeClusterMaker.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloClusterCellLink.h"
#include "CaloUtils/CaloCellList.h"
#include "CaloUtils/CaloClusterStoreHelper.h"
#include "egammaCaloUtils/egammaEnergyPositionAllSamples.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloLayerCalculator.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

egammaLargeClusterMaker::egammaLargeClusterMaker(const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent)
  : AthAlgTool(type, name, parent)
{

  // declare interface
  declareInterface<CaloClusterCollectionProcessor>(this);
}

StatusCode
egammaLargeClusterMaker::initialize()
{
  ATH_CHECK(m_inputClusterCollection.initialize());
  ATH_CHECK(m_cellsKey.initialize());
  ATH_CHECK(m_caloDetDescrMgrKey.initialize());
  return StatusCode::SUCCESS;
}

StatusCode
egammaLargeClusterMaker::execute(const EventContext& ctx,
                                 xAOD::CaloClusterContainer* collection) const
{

  if (!collection) {
    ATH_MSG_ERROR("A null collection was passed, which should never happen");
    return StatusCode::FAILURE;
  }

  SG::ReadHandle<xAOD::CaloClusterContainer> inputClusters(
    m_inputClusterCollection, ctx);
  // retrieve the cell containers
  SG::ReadHandle<CaloCellContainer> cellcoll(m_cellsKey, ctx);

  // retrieve CaloDetDescr
  SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{
    m_caloDetDescrMgrKey, ctx
  };
  ATH_CHECK(caloDetDescrMgrHandle.isValid());

  const CaloDetDescrManager* calodetdescrmgr = *caloDetDescrMgrHandle;

  DataLink<CaloCellContainer> cellLink (cellcoll.ptr(), ctx);

  // The main loop over clusters
  for (const auto* cluster : *inputClusters) {

    if (!m_isFWD) {

      if (cluster->et() < m_centEtThr)
        continue;

      // find the center of the cluster, copying the logic of
      // egammaMiddleShape.cxx

      // check if cluster is in barrel or end-cap
      if (!cluster->inBarrel() && !cluster->inEndcap()) {
        continue;
      }

      // check if cluster is in barrel or end-cap
      bool in_barrel = egammaEnergyPositionAllSamples::inBarrel(*cluster, 2);
      CaloSampling::CaloSample sam = CaloSampling::EMB2;
      if (!in_barrel) {
        sam = CaloSampling::EME2;
      }

      // granularity in (eta,phi) in the pre sampler
      // CaloCellList needs both enums: subCalo and CaloSample
      auto eta = cluster->etaSample(sam);
      auto phi = cluster->phiSample(sam);

      if ((eta == 0. && phi == 0.) || std::abs(eta) > 100) {
        ATH_MSG_WARNING("Weird input cluster, eta = " << eta
                                                      << " phi = " << phi);
        continue;
      }

      // Should get overritten
      bool barrel = false;
      CaloCell_ID::SUBCALO subcalo = CaloCell_ID::LAREM;
      int sampling_or_module = 0;

      CaloDetDescrManager::decode_sample(
        subcalo, barrel, sampling_or_module, (CaloCell_ID::CaloSample)sam);

      // Get the corresponding grannularities : needs to know where you are
      //                  the easiest is to look for the CaloDetDescrElement

      const CaloDetDescrElement* dde = calodetdescrmgr->get_element(
          CaloCell_ID::LAREM, sampling_or_module, barrel, eta, phi);

      // if object does not exist then return
      if (!dde) {
        ATH_MSG_WARNING("Weird input cluster eta = " << eta
                                                     << " phi = " << phi);
        ATH_MSG_WARNING("No detetector element for seeding");
        continue;
      }

      // local granularity
      auto deta = dde->deta();
      auto dphi = dde->dphi();

      // search the hottest cell around the (eta,phi)
      // (eta,phi) are defined as etaSample() and phiSample
      // around this position a hot cell is searched for in a window
      // (m_neta*m_deta,m_nphi*m_dphi), by default (m_neta,m_nphi)=(7,7)
      CaloLayerCalculator calc;
      StatusCode sc =
          calc.fill(*calodetdescrmgr, cellcoll.ptr(), cluster->etaSample(sam),
                    cluster->phiSample(sam), m_neta * deta, m_nphi * dphi,
                    (CaloSampling::CaloSample)sam);
      if (sc.isFailure()) {
        ATH_MSG_WARNING("CaloLayerCalculator failed  fill ");
        continue;
      }
      double etamax = calc.etarmax();
      double phimax = calc.phirmax();

      // create the new cluster
      xAOD::CaloCluster* newCluster =
          CaloClusterStoreHelper::makeCluster(collection, cellLink);
      newCluster->setEta0(etamax);
      newCluster->setPhi0(phimax);
      newCluster->setClusterSize(xAOD::CaloCluster::SW_7_11);

    } else {
      // FWD cluster collection
      if (cluster->et() < m_fwdEtThr){
        continue;
      }

      // check if cluster is in EMEC or FCAL
      if (cluster->inBarrel()) {
        continue;
      }

      // need some positive energy in EME2 or FCAL0 to be a good candidate
      if (cluster->eSample(CaloSampling::EME2)  <= 0 &&
	    cluster->eSample(CaloSampling::FCAL0) <= 0){
        continue;
      }

      // check if cluster is in FCAL or EMEC
      CaloSampling::CaloSample sam = CaloSampling::FCAL0;
      bool in_EMEC = !(xAOD::EgammaHelpers::isFCAL(cluster));
      if(in_EMEC){
        sam = CaloSampling::EME2;
      }
      // granularity in (eta,phi) in the pre sampler
      // CaloCellList needs both enums: subCalo and CaloSample
      auto eta = cluster->etaSample(sam);
      auto phi = cluster->phiSample(sam);

      if ((eta == 0. && phi == 0.) || std::abs(eta) > 100) {
        ATH_MSG_WARNING("Weird input cluster, maxeta = "
                        << eta << " phi = " << phi << " Eeme2 = "
                        << cluster->eSample(CaloSampling::EME2) << " Efcal = "
                        << cluster->eSample(CaloSampling::FCAL0));
        continue;
      }

      // Should get overritten
      bool emec = false;
      CaloCell_ID::SUBCALO subcalo = CaloCell_ID::LARFCAL;
      int sampling_or_module = 0;

      CaloDetDescrManager::decode_sample(
        subcalo, emec, sampling_or_module, (CaloCell_ID::CaloSample)sam);

      // Get the corresponding grannularities : needs to know where you are
      //                  the easiest is to look for the CaloDetDescrElement
      //    const CaloDetDescrElement* get_element_FCAL (const
      //    CaloDetDescriptor* reg, double eta, double phi) const;
      const CaloDetDescrElement* dde = calodetdescrmgr->get_element(
        subcalo, sampling_or_module, emec, eta, phi);

      // if object does not exist then return
      if (!dde) {
        ATH_MSG_WARNING("Weird input cluster eta = " << eta
                                                     << " phi = " << phi);
        ATH_MSG_WARNING("No detetector element for seeding");
        continue;
      }

      // local granularity
      auto deta = dde->deta();
      auto dphi = dde->dphi();

      // search the hottest cell around the (eta,phi)
      // (eta,phi) are defined as etaSample() and phiSample
      // around this position a hot cell is searched for in a window
      // (m_neta*m_deta,m_nphi*m_dphi), by default (m_neta,m_nphi)=(6,6)
      // for EMEC-OUTER FWD much bigger cell size

      // create the new cluster
      xAOD::CaloCluster* newCluster =
          CaloClusterStoreHelper::makeCluster(collection, cellLink);

      // if In EMEC
      CaloLayerCalculator calc;
      StatusCode sc = calc.fill(
          *calodetdescrmgr, cellcoll.ptr(), cluster->etaSample(sam),
          cluster->phiSample(sam), m_netaFWD * deta, m_nphiFWD * dphi,
          (CaloSampling::CaloSample)sam, in_EMEC ? newCluster : nullptr);

      if (sc.isFailure()) {
        ATH_MSG_WARNING("CaloLayerCalculator failed  fill for FWD cluster");
        continue;
      }
      double etamax = calc.etarmax();
      double phimax = calc.phirmax();

      newCluster->setEta0(etamax);
      newCluster->setPhi0(phimax);

      std::map<CaloSampling::CaloSample, double> caloSamV;
      if (!in_EMEC) {
        caloSamV[CaloSampling::FCAL0] = m_drFWD;
      }
      if (m_addCellsFromOtherSamplings) {
        caloSamV[CaloSampling::EME1] = m_drEM;
        if (in_EMEC) {
          caloSamV[CaloSampling::FCAL0] = m_drFWD;
        } else {
          caloSamV[CaloSampling::EME2] = m_drFWD;
        }
      }

      if (!caloSamV.empty()) {
        // If FCAL need to add cell to cluster in a cone.
        // Also if we want cells from other samplings
        std::vector<const CaloCell*> cells;
        cells.reserve(300);
        CaloCellList myList(calodetdescrmgr, cellcoll.ptr());
        for (auto s : caloSamV) {
          myList.select(cluster->etaSample(sam), cluster->phiSample(sam),
                        s.second, (CaloSampling::CaloSample)s.first);
          cells.insert(cells.end(), myList.begin(), myList.end());
        }
        for (const auto* cell : cells) {
          if (!cell || !cell->caloDDE()) {
            continue;
          }
          int index = cellcoll.ptr()->findIndex(cell->caloDDE()->calo_hash());
          if (index == -1) {
            continue;
          }
          newCluster->addCell(index, 1.);
        }
      }
    }  // end isFWD
  }    // main loop over clusters
  return StatusCode::SUCCESS;
}
