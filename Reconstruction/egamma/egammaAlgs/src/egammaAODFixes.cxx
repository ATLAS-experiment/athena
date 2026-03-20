/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaAODFixes.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonAuxContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"

#include "egammaUtils/egAmbLinkHelper.h"
#include "egammaUtils/EMFourMomBuilder.h"
#include "egammaCaloUtils/findMaxECell.h"
#include "egammaCaloUtils/egammaClusterHelpers.h"

#include "CaloUtils/CaloClusterStoreHelper.h"

egammaAODFixes::egammaAODFixes(const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode
egammaAODFixes::initialize()
{
  // The main tool
  if (m_tpetcFix) {
    ATH_CHECK(m_egammaCellRecoveryTool.retrieve());
    // The calo cell need to be there
    ATH_CHECK(m_CaloCellsKey.initialize());
    if (m_correctCluster) {
      ATH_CHECK(m_egClusOutputKey.initialize());
      ATH_CHECK(m_egClusInputKey.initialize());
      ATH_CHECK(m_caloDetDescrMgrKey.initialize());
      ATH_CHECK(m_clusterCorrectionTool.retrieve());
      ATH_CHECK(m_MVACalibSvc.retrieve());
      ATH_CHECK(m_IsoLeakCorrectionTool.retrieve());
      m_egClusCellLinkOutputKey =
	m_egClusOutputKey.key() + "_links";
      ATH_CHECK(m_egClusCellLinkOutputKey.initialize());
    }
  }

  // the data handle keys
  ATH_CHECK(m_electronOutputKey.initialize());
  ATH_CHECK(m_electronInputKey.initialize());
  ATH_CHECK(m_photonOutputKey.initialize());
  ATH_CHECK(m_photonInputKey.initialize());
    
  return StatusCode::SUCCESS;
}

StatusCode
egammaAODFixes::execute(const EventContext& ctx) const {

  static const SG::AuxElement::Accessor<float> acce2("Eadded_Lr2");
  static const SG::AuxElement::Accessor<float> acce3("Eadded_Lr3");
  static const SG::AuxElement::Accessor<float> acce2b("Eadded_Lr2b");
  static const SG::AuxElement::Accessor<float> acce3b("Eadded_Lr3b");
  static const SG::AuxElement::Accessor<float> acce2e("Eadded_Lr2e");
  static const SG::AuxElement::Accessor<float> acce3e("Eadded_Lr3e");

  SG::ReadHandle<xAOD::ElectronContainer> el_inputContainer(m_electronInputKey,ctx);
  SG::WriteHandle<xAOD::ElectronContainer> el_outputContainer(m_electronOutputKey,ctx);
  SG::ReadHandle<xAOD::PhotonContainer> ph_inputContainer(m_photonInputKey,ctx);
  SG::WriteHandle<xAOD::PhotonContainer> ph_outputContainer(m_photonOutputKey,ctx);

  ATH_CHECK(el_outputContainer.record(std::make_unique<xAOD::ElectronContainer>(),
				      std::make_unique<xAOD::ElectronAuxContainer>()));

  ATH_CHECK(ph_outputContainer.record(std::make_unique<xAOD::PhotonContainer>(),
				      std::make_unique<xAOD::PhotonAuxContainer>()));

  xAOD::ElectronContainer* electrons = el_outputContainer.ptr();
  xAOD::PhotonContainer* photons = ph_outputContainer.ptr();
  electrons->reserve(el_inputContainer->size());
  photons->reserve(ph_inputContainer->size());

  ATH_MSG_VERBOSE("Running on " << el_inputContainer->size() << " electrons"
		  " and " << ph_inputContainer->size() << " photons");

  std::map<size_t,std::pair<const xAOD::Egamma*,IegammaCellRecoveryTool::Info>>
    icegRecoveryInfo;

  // Whatever the fix : copy old container content into a new one
  for (const xAOD::Electron* old_el : *el_inputContainer) {
    xAOD::Electron* electron = electrons->push_back(std::make_unique<xAOD::Electron>());
    *electron=*old_el;

    if (m_tpetcFix) {
      float aET = 0;
      IegammaCellRecoveryTool::Info info{};
      if (getCorrectionC(old_el,aET,info).isFailure()) {
	ATH_MSG_WARNING("Failed to recover energies for electron "
			<< old_el->index() << " pT = " << old_el->pt());
      }
      for (auto t : { xAOD::Iso::topoetcone20, xAOD::Iso::topoetcone30,
	    xAOD::Iso::topoetcone40 }) {
	float oiso = 9e9;
	if (!old_el->isolation(oiso,t)) {
	  ATH_MSG_WARNING("Electron " << old_el->index() << " has no "
			  << xAOD::Iso::toCString(t) << " isolation");
	}
	electron->setIsolation(oiso+aET, t);
      }
      std::vector<float> layerEnergies(4);
      getLayerE(info.addedCells,layerEnergies);
      // Legacy info
      acce2(*electron) = layerEnergies[0]+layerEnergies[2];
      acce3(*electron) = layerEnergies[1]+layerEnergies[3];
      // more granular for debug
      acce2b(*electron) = layerEnergies[0];
      acce3b(*electron) = layerEnergies[1];
      acce2e(*electron) = layerEnergies[2];
      acce3e(*electron) = layerEnergies[3];

      if (m_correctCluster) {
	icegRecoveryInfo[old_el->caloCluster()->index()] =
	  std::make_pair(old_el,info);
      }
    }
  }
  
  for (const xAOD::Photon* old_ph : *ph_inputContainer) {
    xAOD::Photon* photon = photons->push_back(std::make_unique<xAOD::Photon>());
    *photon=*old_ph;

    if (m_tpetcFix) {
      float aET = 0;
      IegammaCellRecoveryTool::Info info{};
      if (getCorrectionC(old_ph,aET,info).isFailure()) {
	ATH_MSG_WARNING("Failed to recover energies for photon "
			<< old_ph->index() << " pT = " << old_ph->pt());
      }
      for (auto t : { xAOD::Iso::topoetcone20, xAOD::Iso::topoetcone30,
	    xAOD::Iso::topoetcone40 }) {
	float oiso = 9e9;
	if (!old_ph->isolation(oiso,t)) {
	  ATH_MSG_WARNING("Photon " << old_ph->index() << " has no "
			  << xAOD::Iso::toCString(t) << " isolation");
	}
	photon->setIsolation(oiso+aET, t);
      }
      std::vector<float> layerEnergies(4);
      getLayerE(info.addedCells,layerEnergies);
      // legacy info
      acce2(*photon) = layerEnergies[0]+layerEnergies[2];
      acce3(*photon) = layerEnergies[1]+layerEnergies[3];
      // more granular for debug
      acce2b(*photon) = layerEnergies[0];
      acce3b(*photon) = layerEnergies[1];
      acce2e(*photon) = layerEnergies[2];
      acce3e(*photon) = layerEnergies[3];

      if (m_correctCluster) {
	icegRecoveryInfo[old_ph->caloCluster()->index()] =
	  std::make_pair(old_ph,info);
      }
    }
  }

  // Ambiguity link fix
  if (m_ambiguityFix) {
    egAmbLinkHelper::doAmbiguityLinks(ctx, electrons, photons);
    egAmbLinkHelper::doAmbiguityLinks(ctx, photons, electrons);
  }

  // Correct also layer energies (L2 and L3) and in addition the corresponding
  // cluster energy (raw, alt, cal)
  if (m_correctCluster) {

    // The calo Det Descr manager
    SG::ReadCondHandle<CaloDetDescrManager> caloDetDescrMgrHandle{
      m_caloDetDescrMgrKey, ctx
	};
    ATH_CHECK(caloDetDescrMgrHandle.isValid());
    const CaloDetDescrManager* mgr = *caloDetDescrMgrHandle;

    static const std::vector<xAOD::CaloCluster::CaloSample> caloSam{
      CaloSampling::EMB2, CaloSampling::EMB3,
      CaloSampling::EME2, CaloSampling::EME3 };

    SG::ReadHandle<xAOD::CaloClusterContainer> egcl_inputContainer(m_egClusInputKey,ctx);

    SG::WriteHandle<CaloClusterCellLinkContainer> egcl_CellLinkContainer(
	   m_egClusCellLinkOutputKey, ctx);
    ATH_CHECK(egcl_CellLinkContainer.record(
	   std::make_unique<CaloClusterCellLinkContainer>()));

    SG::WriteHandle<xAOD::CaloClusterContainer> egcl_outputContainer(m_egClusOutputKey,ctx);
    ATH_CHECK(egcl_outputContainer.record(std::make_unique<xAOD::CaloClusterContainer>(),
					  std::make_unique<xAOD::CaloClusterAuxContainer>()));

    xAOD::CaloClusterContainer* egClusters = egcl_outputContainer.ptr();
    egClusters->reserve(egcl_inputContainer->size());

    for (const xAOD::CaloCluster* old_egcl : *egcl_inputContainer) {
      xAOD::CaloCluster* cluster =
	egClusters->push_back(std::make_unique<xAOD::CaloCluster>());
      *cluster=*old_egcl;

      ATH_MSG_VERBOSE("cluster ptrs old " << old_egcl << " new " << cluster
		      << " indices " << old_egcl->index() << " " << cluster->index()); 

      size_t index = old_egcl->index();
      std::vector<const CaloCell*> cc;
      if (icegRecoveryInfo.find(index) != icegRecoveryInfo.end()) {
	cc = icegRecoveryInfo[index].second.addedCells;
      } else {
	ATH_MSG_WARNING("Could not find added calo cells"
			" for cluster with index " << index);
	return StatusCode::FAILURE;
      }

      bool doAddCell_condition1 = cc.size() > 0;
      const xAOD::Egamma* eg = icegRecoveryInfo[index].first;
      float addedE2 = icegRecoveryInfo[index].second.eCells[0];
      bool doAddCell_condition2 = addedE2 > 0 ||
	addedE2 + old_egcl->energyBE(2) > 0;
      if (!doAddCell_condition2) {
	ATH_MSG_VERBOSE(
	  "Cluster should be corrected, but the additional energy in S2 "
	  << addedE2 << " is too negative w.r.t. original one " <<
	  old_egcl->energyBE(2));
      }
      if (doAddCell_condition1 && doAddCell_condition2) {
	ATH_MSG_VERBOSE("Number of cells " << old_egcl->getCellLinks()->size()
			<< " " << cluster->getCellLinks()->size());

	const CaloClusterCellLink* cellLinks = cluster->getOwnCellLinks();
	CaloClusterCellLink::const_iterator cellItr = cellLinks->begin();
	CaloClusterCellLink::const_iterator cellEnd = cellLinks->end();
	for (; cellItr != cellEnd; ++cellItr) {
	  ATH_MSG_VERBOSE("A cell in the new cluster w = " << cellItr.weight()
			  << " e = " << cellItr->e());
	}
	const CaloClusterCellLink* ocellLinks = old_egcl->getCellLinks();
	cellItr = ocellLinks->begin();
	cellEnd = ocellLinks->end();
	for (; cellItr != cellEnd; ++cellItr) {
	  ATH_MSG_VERBOSE("A cell in the old cluster w = " << cellItr.weight()
			  << " e = " << cellItr->e());
	}

	const CaloCellContainer* inputcells =
	  cluster->getCellLinks()->getCellContainer();
	for (const auto *cell : cc) {
	  int cindex = inputcells->findIndex(cell->caloDDE()->calo_hash());
	  cluster->addCell(cindex,1.);
	}
	ATH_MSG_VERBOSE("After adding the cells, number of cells "
			<< cellLinks->size());
	cellItr = cellLinks->begin();
	cellEnd = cellLinks->end();
	for (; cellItr != cellEnd; ++cellItr) {
	  ATH_MSG_VERBOSE("After addding a cell in the new cluster w = "
			  << cellItr.weight()
			  << " e = " << cellItr->e());
	}
	CaloClusterKineHelper::calculateKine(cluster, true, true);

	egammaClusterHelpers::refineEta1Position(cluster, *mgr);
	// Save the state before the corrections
	cluster->setAltE(cluster->e());
	cluster->setAltEta(cluster->eta());
	cluster->setAltPhi(cluster->phi());

	xAOD::EgammaParameters::EgammaType egType =
	  xAOD::EgammaParameters::electron;
	if (xAOD::EgammaHelpers::isConvertedPhoton(eg)) {
	  egType = xAOD::EgammaParameters::convertedPhoton;
	} else if (xAOD::EgammaHelpers::isPhoton(eg)) {
	  egType = xAOD::EgammaParameters::unconvertedPhoton;
	}
	ATH_CHECK(m_clusterCorrectionTool->execute(
	 ctx, cluster, egType, xAOD::EgammaHelpers::isBarrel(cluster)));
	cluster->setRawE(cluster->e());
	cluster->setRawEta(cluster->eta());
	cluster->setRawPhi(cluster->phi());
	//
	egammaClusterHelpers::fillPositionsInCalo(cluster, *mgr);
	//
	if (m_MVACalibSvc->execute(*cluster,
				   *eg).isFailure()) {
	  ATH_MSG_ERROR("Problem executing MVA cluster tool");
	}
      }
    }
    CaloClusterStoreHelper::finalizeClusters(
      ctx,
      egcl_outputContainer,
      egcl_CellLinkContainer);
    rebuildLink(photons, egClusters, ctx);
    rebuildLink(electrons, egClusters, ctx);
    EMFourMomBuilder::calculate(electrons);
    EMFourMomBuilder::calculate(photons);
  }

  return StatusCode::SUCCESS;
}

StatusCode egammaAODFixes::getCorrection(
  const xAOD::Egamma *eg,
  float& aET,
  std::vector<float>& lE) const {

  const xAOD::CaloCluster *clus = eg->caloCluster();
  egammaCellUtils::MaxECell maxECell(clus);
  if (maxECell.sc == StatusCode::FAILURE) {
    ATH_MSG_WARNING("Issues in finding maximum energy cell");
    return maxECell.sc;
  }
  IegammaCellRecoveryTool::Info info{};
  info.etamax = maxECell.etaCell;
  info.phimax = maxECell.phiCell;
  if (m_egammaCellRecoveryTool->execute(*clus,info).isFailure()) {
    ATH_MSG_WARNING("Issue trying to recover cells");
  }
  double aE2 = info.eCells[0];
  double aE3 = info.eCells[1];
  std::string type = (eg->type() == xAODType::Photon) ? "Photon " :
    ((eg->type() == xAODType::Electron) ? "Electron " : "Unknown ");
  ATH_MSG_VERBOSE(type << eg->index()
		  << " added Energies in Layer 2 " << aE2 << " and 3 " << aE3
		  << " cluster index = " << clus->index());
  double iceta = 1./std::cosh(clus->etaBE(2));
  aET = (aE2+aE3)*iceta;
  if (m_correctCluster) {
    getLayerE(info.addedCells, lE);
    ATH_MSG_VERBOSE("Per layer "
		    << lE[0] << " " << lE[1] << " "
		    << lE[2] << " " << lE[3]);
  }
  return StatusCode::SUCCESS;
}

StatusCode egammaAODFixes::getCorrectionC(
  const xAOD::Egamma *eg,
  float& aET,
  IegammaCellRecoveryTool::Info &info) const {

  const xAOD::CaloCluster *clus = eg->caloCluster();
  egammaCellUtils::MaxECell maxECell(clus);
  if (maxECell.sc == StatusCode::FAILURE) {
    ATH_MSG_WARNING("Issues in finding maximum energy cell");
    return maxECell.sc;
  }
  info.etamax = maxECell.etaCell;
  info.phimax = maxECell.phiCell;
  if (m_egammaCellRecoveryTool->execute(*clus,info).isFailure()) {
    ATH_MSG_WARNING("Issue trying to recover cells");
  }
  double aE2 = info.eCells[0];
  double aE3 = info.eCells[1];
  std::string type = (eg->type() == xAODType::Photon) ? "Photon " :
    ((eg->type() == xAODType::Electron) ? "Electron " : "Unknown ");
  ATH_MSG_VERBOSE(type << eg->index()
		  << " added Energies in Layer 2 " << aE2 << " and 3 " << aE3
		  << " cluster index = " << clus->index());
  double iceta = 1./std::cosh(clus->etaBE(2));
  aET = (aE2+aE3)*iceta;
  return StatusCode::SUCCESS;
}


void egammaAODFixes::getLayerE(
  std::vector<const CaloCell*>& cells,
  std::vector<float>& lE) const
{
  if (lE.size() != 4) {
    ATH_MSG_WARNING("The passed layer energies should have a size 4");
    lE.resize(4);
  }
  for (auto *cell : cells) {
    int layer = cell->caloDDE()->getSampling();
    if (layer == CaloSampling::EMB2)
      lE[0] += cell->e();
    else if (layer == CaloSampling::EMB3)
      lE[1] += cell->e();
    else if (layer == CaloSampling::EME2)
      lE[2] += cell->e();
    else if (layer == CaloSampling::EME3)
      lE[3] += cell->e();
  }
}

void egammaAODFixes::rebuildLink(
  xAOD::EgammaContainer *egContainer,
  xAOD::CaloClusterContainer *egclContainer,
  const EventContext& ctx) const {

  for (xAOD::Egamma *eg : *egContainer) {

    // link the new eg object (update for isolation)
    // to the new cluster (update for layer energies)
    ElementLink<xAOD::CaloClusterContainer> clusterLink(
      *egclContainer, eg->caloCluster()->index(), ctx);
    std::vector<ElementLink<xAOD::CaloClusterContainer>> egClustersLink{
      clusterLink };
    eg->setCaloClusterLinks(egClustersLink);

    // Also update the leakage correction
    // as it depends on the cluster LAr+PS energy
    for (auto t : { xAOD::Iso::topoetcone20, xAOD::Iso::topoetcone30,
	            xAOD::Iso::topoetcone40 }) {
      if (!eg->setIsolationCaloCorrection(
		  m_IsoLeakCorrectionTool->GetPtCorrection(*eg,t),
		  t,xAOD::Iso::ptCorrection)) {
	ATH_MSG_WARNING("could not set leakage correction for isolation type"
			<< xAOD::Iso::toCString(t) << " and object pT, eta = "
			<< eg->pt() << "  " << eg->eta());
      }
    }
  }

}
